#include "vulkan_renderer_layer.hpp"
#ifdef max
	#undef max
#endif

#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <format>
#include <fstream>
#include <limits>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>

#define CHECK_VK(x) \
	do { \
		VkResult res = x; \
		if (res != VK_SUCCESS) { \
			throw std::runtime_error(std::format("Vulkan error: {} at {}:{}", string_VkResult(res), __FILE__, __LINE__)); \
		} \
	} while (0)

namespace {

constexpr auto noTimeout = std::numeric_limits<uint64_t>::max();

const std::vector<Vertex> s_Vertices = {
	{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
	{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } },
	{ { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
};

const std::vector<uint32_t> s_Indices = { 0, 1, 2, 2, 3, 0 };

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	return func ? func(instance, pCreateInfo, pAllocator, pDebugMessenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) {
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
	auto msg = std::format("\n\tVulkan Validation [{}]: {}", string_VkDebugUtilsMessageTypeFlagsEXT(type), pCallbackData->pMessage);
	switch (severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LUMEN_CORE_WARN("{}", msg);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LUMEN_CORE_ERROR("{}", msg);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LUMEN_CORE_INFO("{}", msg);
			break;
		default:
			LUMEN_CORE_TRACE("{}", msg);
			break;
	}
	return VK_FALSE;
}

template <typename AvailableRange, typename NameFn>
std::vector<const char *> FilterSupportedNames(const AvailableRange &available, NameFn getName,
		const std::vector<const char *> &wanted, bool required, std::string_view kind) {
	std::vector<const char *> enabled;
	for (const char *name : wanted) {
		bool found = std::ranges::any_of(available, [&](const auto &item) {
			return std::strcmp(getName(item), name) == 0;
		});
		if (found) {
			enabled.push_back(name);
		} else if (required) {
			throw std::runtime_error(std::format("Required Vulkan {} not supported: {}", kind, name));
		} else {
			LUMEN_CORE_WARN("Optional Vulkan {} not supported: {}", kind, name);
		}
	}
	return enabled;
}

} // namespace

VulkanRendererLayer::VulkanRendererLayer(lm::Window *window) : Layer("VulkanRendererLayer"), m_Window(window) {}

void VulkanRendererLayer::OnAttach() {
	CreateInstance();
	if constexpr (k_EnableValidation) {
		SetupDebugMessenger();
	}
	CreateSurface();
	PickPhysicalDevice();
	CreateDevice();
	CreateSwapchain();
	CreateImageViews();
	CreateDescriptorSetLayout();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateGraphicsPipeline();
	CreateCommandPool();
	CreateCommandBuffers();
	CreateSyncObjects();

	CreateVertexBuffer();
	CreateIndexBuffer();
}

void VulkanRendererLayer::OnDetach() {
	Cleanup();
}

void VulkanRendererLayer::OnEvent(lm::Event &e) {
	lm::EventDispatcher dp(e);
	dp.Dispatch<lm::WindowResizeEvent>([this](auto &e) { return OnWindowResize(e.GetExtent()); });
}

void VulkanRendererLayer::OnRender() {
	CHECK_VK(vkWaitForFences(m_Context.device, 1, &m_Context.inFlightFences[frameIndex], VK_TRUE, noTimeout));

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_Context.device, m_Context.swapchain, noTimeout,
			m_Context.presentCompleteSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapchain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		CHECK_VK(result);
	}

	CHECK_VK(vkResetFences(m_Context.device, 1, &m_Context.inFlightFences[frameIndex]));
	CHECK_VK(vkResetCommandBuffer(m_Context.commandBuffers[frameIndex], 0));

	RecordCommandBuffer(imageIndex);

	UpdateUniformBuffer();

	VkPipelineStageFlags waitDestinationStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_Context.presentCompleteSemaphores[frameIndex],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_Context.commandBuffers[frameIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_Context.renderFinishedSemaphores[imageIndex],
	};
	CHECK_VK(vkQueueSubmit(m_Context.graphicsQueue, 1, &submitInfo, m_Context.inFlightFences[frameIndex]));

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_Context.renderFinishedSemaphores[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &m_Context.swapchain,
		.pImageIndices = &imageIndex,
		.pResults = nullptr,
	};
	result = vkQueuePresentKHR(m_Context.presentQueue, &presentInfo);
	if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || framebufferResized) {
		framebufferResized = false;
		RecreateSwapchain();
	} else {
		CHECK_VK(result);
	}

	frameIndex = (frameIndex + 1) % k_MaxFramesInFlight;
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

bool VulkanRendererLayer::OnWindowResize(lm::WindowExtent /*extent*/) {
	framebufferResized = true;
	return false;
}

void VulkanRendererLayer::UpdateUniformBuffer() {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f), glm::vec3(0.0f), { 0.0f, 0.0f, 1.0f });
	float aspectRatio = static_cast<float>(m_Context.swapchainExtent.width) / m_Context.swapchainExtent.height;
	ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	ubo.proj[1][1] *= -1;
	std::memcpy(m_Context.uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
}

void VulkanRendererLayer::RecreateSwapchain() {
	lm::WindowExtent extent = m_Window->GetExtent();
	while (extent.width == 0 || extent.height == 0) {
		extent = m_Window->GetExtent();
		m_Window->WaitForEvents();
	}

	CHECK_VK(vkDeviceWaitIdle(m_Context.device));

	CleanupSwapchain();

	CreateSwapchain();
	CreateImageViews();
}

void VulkanRendererLayer::CleanupSwapchain() {
	for (auto imageView : m_Context.swapchainImageViews) {
		vkDestroyImageView(m_Context.device, imageView, nullptr);
	}
	m_Context.swapchainImageViews.clear();
	vkDestroySwapchainKHR(m_Context.device, m_Context.swapchain, nullptr);
	m_Context.swapchain = VK_NULL_HANDLE;
}

void VulkanRendererLayer::CreateInstance() {
	constexpr VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "VulkanRendererLayer",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Lumen Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = k_RequiredVulkanVersion,
	};

	auto enabledLayers = FilterSupportedInstanceLayers(k_RequiredInstanceLayers, /*required=*/true);
	auto enabledExtensions = FilterSupportedInstanceExtensions(k_RequiredInstanceExtensions, /*required=*/true);

	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
		.ppEnabledLayerNames = enabledLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
	};

	CHECK_VK(vkCreateInstance(&createInfo, nullptr, &m_Context.instance));
}

void VulkanRendererLayer::SetupDebugMessenger() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugCallback,
	};
	CHECK_VK(CreateDebugUtilsMessengerEXT(m_Context.instance, &createInfo, nullptr, &m_Context.debugMessenger));
}

void VulkanRendererLayer::CreateSurface() {
	HWND handle = static_cast<HWND>(m_Window->GetNativeHandle());
	VkWin32SurfaceCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hwnd = handle,
	};
	CHECK_VK(vkCreateWin32SurfaceKHR(m_Context.instance, &createInfo, nullptr, &m_Context.surface));
}

void VulkanRendererLayer::PickPhysicalDevice() {
	auto devices = EnumeratePhysicalDevices();
	if (devices.empty()) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support.");
	}

	auto it = std::ranges::find_if(devices, [this](VkPhysicalDevice d) { return IsDeviceSuitable(d); });
	if (it == devices.end()) {
		throw std::runtime_error("Failed to find a suitable GPU.");
	}
	m_Context.physicalDevice = *it;

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(m_Context.physicalDevice, &props);
	LUMEN_CORE_INFO("Using GPU: {}", props.deviceName);
}

void VulkanRendererLayer::CreateDevice() {
	m_Context.queueFamilyIndices = FindQueueFamilies(m_Context.physicalDevice);
	const QueueFamilyIndices &indices = m_Context.queueFamilyIndices;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value(),
		indices.computeFamily.value(),
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
		});
	}

	auto enabledExtensions = FilterSupportedDeviceExtensions(m_Context.physicalDevice, k_RequiredDeviceExtensions, /*required=*/true);

	DeviceFeatureChain features;
	QueryDeviceFeatures(m_Context.physicalDevice, features);
	features.vulkan11Features.shaderDrawParameters = VK_TRUE;
	features.vulkan13Features.dynamicRendering = VK_TRUE;
	features.extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features.features2,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
		.pEnabledFeatures = nullptr,
	};

	CHECK_VK(vkCreateDevice(m_Context.physicalDevice, &createInfo, nullptr, &m_Context.device));

	vkGetDeviceQueue(m_Context.device, indices.graphicsFamily.value(), 0, &m_Context.graphicsQueue);
	vkGetDeviceQueue(m_Context.device, indices.presentFamily.value(), 0, &m_Context.presentQueue);
	vkGetDeviceQueue(m_Context.device, indices.computeFamily.value(), 0, &m_Context.computeQueue);
}

void VulkanRendererLayer::CreateSwapchain() {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	CHECK_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context.physicalDevice, m_Context.surface, &surfaceCapabilities));
	m_Context.swapchainExtent = ChooseSwapExtent(surfaceCapabilities);
	uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

	uint32_t formatCount = 0;
	CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context.physicalDevice, m_Context.surface, &formatCount, nullptr));
	std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
	CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context.physicalDevice, m_Context.surface, &formatCount, availableFormats.data()));
	m_Context.swapchainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

	uint32_t presentModeCount = 0;
	CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context.physicalDevice, m_Context.surface, &presentModeCount, nullptr));
	std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
	CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context.physicalDevice, m_Context.surface, &presentModeCount, availablePresentModes.data()));
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(availablePresentModes);

	const QueueFamilyIndices &indices = m_Context.queueFamilyIndices;
	std::array<uint32_t, 2> queueFamilyIndices = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value(),
	};
	bool sameFamily = indices.graphicsFamily.value() == indices.presentFamily.value();

	VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = m_Context.surface,
		.minImageCount = minImageCount,
		.imageFormat = m_Context.swapchainSurfaceFormat.format,
		.imageColorSpace = m_Context.swapchainSurfaceFormat.colorSpace,
		.imageExtent = m_Context.swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = sameFamily ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
		.queueFamilyIndexCount = sameFamily ? 0 : static_cast<uint32_t>(queueFamilyIndices.size()),
		.pQueueFamilyIndices = sameFamily ? nullptr : queueFamilyIndices.data(),
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = m_Context.swapchain,
	};
	CHECK_VK(vkCreateSwapchainKHR(m_Context.device, &createInfo, nullptr, &m_Context.swapchain));

	uint32_t swapchainImageCount;
	vkGetSwapchainImagesKHR(m_Context.device, m_Context.swapchain, &swapchainImageCount, nullptr);
	m_Context.swapchainImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(m_Context.device, m_Context.swapchain, &swapchainImageCount, m_Context.swapchainImages.data());
}

void VulkanRendererLayer::CreateImageViews() {
	assert(m_Context.swapchainImageViews.empty());
	VkImageViewCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = m_Context.swapchainSurfaceFormat.format,
		.subresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
		},
	};

	size_t imageCount = m_Context.swapchainImages.size();
	m_Context.swapchainImageViews.resize(imageCount);
	for (size_t i = 0; i < imageCount; ++i) {
		createInfo.image = m_Context.swapchainImages[i];
		vkCreateImageView(m_Context.device, &createInfo, nullptr, &m_Context.swapchainImageViews[i]);
	}
}

void VulkanRendererLayer::CreateGraphicsPipeline() {
	VkShaderModule shaderModule = CreateShaderModule(readSpirV("shaders/shader.spv"));
	VkPipelineShaderStageCreateInfo vertShaderStage{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = shaderModule,
		.pName = "vertMain",
	};
	VkPipelineShaderStageCreateInfo fragShaderStage{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = shaderModule,
		.pName = "fragMain",
	};
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStage, fragShaderStage };

	// Vertex input
	VkVertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = Vertex::GetAttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexInputState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data(),
	};

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f,
	};

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
	};

	// Depth Stencil
	nullptr;

	// Color blend attachemnts
	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = VK_FALSE,
		.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};

	// Dynamic state
	std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &m_Context.descriptorSetLayout,
		.pushConstantRangeCount = 0,
	};
	CHECK_VK(vkCreatePipelineLayout(m_Context.device, &pipelineLayoutInfo, nullptr, &m_Context.graphicsPipelineLayout));

	// Dynamic rendering
	VkPipelineRenderingCreateInfo pipelineRenderingInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &m_Context.swapchainSurfaceFormat.format,
	};

	VkGraphicsPipelineCreateInfo graphicsPipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipelineRenderingInfo,
		.stageCount = static_cast<uint32_t>(shaderStages.size()),
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInputState,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = m_Context.graphicsPipelineLayout,
		.renderPass = nullptr,
	};
	CHECK_VK(vkCreateGraphicsPipelines(m_Context.device, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &m_Context.graphicsPipeline));

	vkDestroyShaderModule(m_Context.device, shaderModule, nullptr);
}

void VulkanRendererLayer::CreateCommandPool() {
	VkCommandPoolCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = m_Context.queueFamilyIndices.graphicsFamily.value(),
	};
	CHECK_VK(vkCreateCommandPool(m_Context.device, &createInfo, nullptr, &m_Context.commandPool));
}

void VulkanRendererLayer::CreateCommandBuffers() {
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_Context.commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = k_MaxFramesInFlight,
	};
	m_Context.commandBuffers.resize(k_MaxFramesInFlight);
	CHECK_VK(vkAllocateCommandBuffers(m_Context.device, &allocInfo, m_Context.commandBuffers.data()));
}

void VulkanRendererLayer::CreateSyncObjects() {
	assert(m_Context.presentCompleteSemaphores.empty() &&
			m_Context.renderFinishedSemaphores.empty() &&
			m_Context.inFlightFences.empty());

	VkSemaphoreCreateInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	const size_t swapchainImageCount = m_Context.swapchainImages.size();
	m_Context.renderFinishedSemaphores.resize(swapchainImageCount);
	for (size_t i = 0; i < swapchainImageCount; ++i) {
		CHECK_VK(vkCreateSemaphore(m_Context.device, &semaphoreInfo, nullptr, &m_Context.renderFinishedSemaphores[i]));
	}

	m_Context.presentCompleteSemaphores.resize(k_MaxFramesInFlight);
	m_Context.inFlightFences.resize(k_MaxFramesInFlight);
	for (size_t i = 0; i < k_MaxFramesInFlight; ++i) {
		CHECK_VK(vkCreateSemaphore(m_Context.device, &semaphoreInfo, nullptr, &m_Context.presentCompleteSemaphores[i]));

		VkFenceCreateInfo fenceInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };
		CHECK_VK(vkCreateFence(m_Context.device, &fenceInfo, nullptr, &m_Context.inFlightFences[i]));
	}
}

void VulkanRendererLayer::CreateVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(s_Vertices[0]) * s_Vertices.size();
	auto [stagingBuffer, stagingBufferMemory] = CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void *data;
	CHECK_VK(vkMapMemory(m_Context.device, stagingBufferMemory, 0, bufferSize, 0, &data));
	std::memcpy(data, s_Vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(m_Context.device, stagingBufferMemory);

	std::tie(m_Context.vertexBuffer, m_Context.vertexBufferMemory) = CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
					VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	CopyBuffer(m_Context.vertexBuffer, stagingBuffer, bufferSize);

	vkFreeMemory(m_Context.device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(m_Context.device, stagingBuffer, nullptr);
}

void VulkanRendererLayer::CreateIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(s_Indices[0]) * s_Indices.size();
	auto [stagingBuffer, stagingBufferMemory] = CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void *data;
	CHECK_VK(vkMapMemory(m_Context.device, stagingBufferMemory, 0, bufferSize, 0, &data));
	std::memcpy(data, s_Indices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(m_Context.device, stagingBufferMemory);

	std::tie(m_Context.indexBuffer, m_Context.indexBufferMemory) = CreateBuffer(
			bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	CopyBuffer(m_Context.indexBuffer, stagingBuffer, bufferSize);

	vkFreeMemory(m_Context.device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(m_Context.device, stagingBuffer, nullptr);
}

void VulkanRendererLayer::CreateUniformBuffers() {
	m_Context.uniformBuffers.resize(k_MaxFramesInFlight);
	m_Context.uniformBuffersMemory.resize(k_MaxFramesInFlight);
	m_Context.uniformBuffersMapped.resize(k_MaxFramesInFlight);
	for (size_t i = 0; i < k_MaxFramesInFlight; ++i) {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		auto [buffer, memory] = CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		m_Context.uniformBuffers[i] = buffer;
		m_Context.uniformBuffersMemory[i] = memory;
		CHECK_VK(vkMapMemory(m_Context.device, memory, 0, bufferSize, 0, &m_Context.uniformBuffersMapped[i]));
	}
}

void VulkanRendererLayer::CreateDescriptorPool() {
	VkDescriptorPoolSize poolSize{};
}

void VulkanRendererLayer::CreateDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
	};
	VkDescriptorSetLayoutCreateInfo layoutInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &uboLayoutBinding,
	};
	CHECK_VK(vkCreateDescriptorSetLayout(m_Context.device, &layoutInfo, nullptr, &m_Context.descriptorSetLayout));
}

void VulkanRendererLayer::Cleanup() {
	CHECK_VK(vkDeviceWaitIdle(m_Context.device));

	vkFreeMemory(m_Context.device, m_Context.indexBufferMemory, nullptr);
	vkDestroyBuffer(m_Context.device, m_Context.indexBuffer, nullptr);

	vkFreeMemory(m_Context.device, m_Context.vertexBufferMemory, nullptr);
	vkDestroyBuffer(m_Context.device, m_Context.vertexBuffer, nullptr);

	for (size_t i = 0; i < k_MaxFramesInFlight; ++i) {
		vkDestroyFence(m_Context.device, m_Context.inFlightFences[i], nullptr);
		vkDestroySemaphore(m_Context.device, m_Context.presentCompleteSemaphores[i], nullptr);
	}
	m_Context.inFlightFences.clear();
	m_Context.presentCompleteSemaphores.clear();
	for (size_t i = 0; i < m_Context.swapchainImages.size(); ++i) {
		vkDestroySemaphore(m_Context.device, m_Context.renderFinishedSemaphores[i], nullptr);
	}
	m_Context.renderFinishedSemaphores.clear();
	vkDestroyCommandPool(m_Context.device, m_Context.commandPool, nullptr);
	vkDestroyPipeline(m_Context.device, m_Context.graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_Context.device, m_Context.graphicsPipelineLayout, nullptr);
	CleanupSwapchain();
	vkDestroyDevice(m_Context.device, nullptr);
	vkDestroySurfaceKHR(m_Context.instance, m_Context.surface, nullptr);
	if (m_Context.debugMessenger != VK_NULL_HANDLE) {
		DestroyDebugUtilsMessengerEXT(m_Context.instance, m_Context.debugMessenger, nullptr);
	}
	vkDestroyInstance(m_Context.instance, nullptr);
}

// ---------------------------------------------------------------------------
// Query
// ---------------------------------------------------------------------------

std::vector<VkLayerProperties> VulkanRendererLayer::EnumerateInstanceLayers() const {
	uint32_t count = 0;
	CHECK_VK(vkEnumerateInstanceLayerProperties(&count, nullptr));
	std::vector<VkLayerProperties> available(count);
	CHECK_VK(vkEnumerateInstanceLayerProperties(&count, available.data()));
	return available;
}

std::vector<VkExtensionProperties> VulkanRendererLayer::EnumerateInstanceExtensions() const {
	uint32_t count = 0;
	CHECK_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
	std::vector<VkExtensionProperties> available(count);
	CHECK_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data()));
	return available;
}

std::vector<VkPhysicalDevice> VulkanRendererLayer::EnumeratePhysicalDevices() const {
	uint32_t count = 0;
	CHECK_VK(vkEnumeratePhysicalDevices(m_Context.instance, &count, nullptr));
	std::vector<VkPhysicalDevice> devices(count);
	CHECK_VK(vkEnumeratePhysicalDevices(m_Context.instance, &count, devices.data()));
	return devices;
}

std::vector<VkExtensionProperties> VulkanRendererLayer::EnumerateDeviceExtensions(VkPhysicalDevice device) const {
	uint32_t count = 0;
	CHECK_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr));
	std::vector<VkExtensionProperties> available(count);
	CHECK_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data()));
	return available;
}

// ---------------------------------------------------------------------------
// Filter
// ---------------------------------------------------------------------------

std::vector<const char *> VulkanRendererLayer::FilterSupportedInstanceLayers(
		const std::vector<const char *> &wanted, bool required) const {
	return FilterSupportedNames(EnumerateInstanceLayers(), [](const VkLayerProperties &p) { return p.layerName; }, wanted, required, "layer");
}

std::vector<const char *> VulkanRendererLayer::FilterSupportedInstanceExtensions(
		const std::vector<const char *> &wanted, bool required) const {
	return FilterSupportedNames(EnumerateInstanceExtensions(), [](const VkExtensionProperties &p) { return p.extensionName; }, wanted, required, "instance extension");
}

std::vector<const char *> VulkanRendererLayer::FilterSupportedDeviceExtensions(
		VkPhysicalDevice device, const std::vector<const char *> &wanted, bool required) const {
	return FilterSupportedNames(EnumerateDeviceExtensions(device), [](const VkExtensionProperties &p) { return p.extensionName; }, wanted, required, "device extension");
}

// ---------------------------------------------------------------------------
// Device suitability
// ---------------------------------------------------------------------------

void VulkanRendererLayer::QueryDeviceFeatures(VkPhysicalDevice device, DeviceFeatureChain &outChain) const {
	outChain.Link();
	vkGetPhysicalDeviceFeatures2(device, &outChain.features2);
}

bool VulkanRendererLayer::SupportsRequiredDeviceExtensions(VkPhysicalDevice device) const {
	auto available = EnumerateDeviceExtensions(device);
	return std::ranges::all_of(k_RequiredDeviceExtensions, [&](const char *name) {
		return std::ranges::any_of(available, [name](const auto &p) { return std::strcmp(p.extensionName, name) == 0; });
	});
}

bool VulkanRendererLayer::IsDeviceSuitable(VkPhysicalDevice device) const {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(device, &props);
	bool supportsVersion = props.apiVersion >= k_RequiredVulkanVersion;

	if (!SupportsRequiredDeviceExtensions(device)) {
		return false;
	}

	DeviceFeatureChain features;
	QueryDeviceFeatures(device, features);
	bool supportsFeatures =
			features.vulkan11Features.shaderDrawParameters &&
			features.vulkan13Features.dynamicRendering &&
			features.extendedDynamicStateFeatures.extendedDynamicState;

	return supportsVersion && supportsFeatures && FindQueueFamilies(device).IsComplete();
}

QueueFamilyIndices VulkanRendererLayer::FindQueueFamilies(VkPhysicalDevice device) const {
	QueueFamilyIndices indices;

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

	bool foundCombinedGraphicsPresent = false;
	bool foundDedicatedCompute = false;

	for (uint32_t i = 0; i < count; ++i) {
		const auto &family = families[i];
		bool hasGraphics = (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
		bool hasCompute = (family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;

		VkBool32 presentSupport = VK_FALSE;
		CHECK_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Context.surface, &presentSupport));

		// Graphics fallback
		if (!foundCombinedGraphicsPresent && !indices.graphicsFamily.has_value() && hasGraphics) {
			indices.graphicsFamily = i;
		}

		// Present fallback
		if (!foundCombinedGraphicsPresent && presentSupport && !indices.presentFamily.has_value()) {
			indices.presentFamily = i;
		}

		// Prefered Graphics & Present
		if (!foundCombinedGraphicsPresent && hasGraphics && presentSupport) {
			indices.graphicsFamily = i;
			indices.presentFamily = i;
			foundCombinedGraphicsPresent = true;
		}

		// Compute
		if (!foundDedicatedCompute && hasCompute && !hasGraphics) {
			indices.computeFamily = i;
			foundDedicatedCompute = true;
		} else if (!foundDedicatedCompute && !indices.computeFamily.has_value() && hasCompute) {
			indices.computeFamily = i;
		}
	}

	return indices;
}

VkSurfaceFormatKHR VulkanRendererLayer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const {
	const auto formatIt = std::ranges::find_if(availableFormats, [](const auto &fmt) {
		return fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	});
	return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

VkPresentModeKHR VulkanRendererLayer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const {
	assert(std::ranges::any_of(availablePresentModes, [](const VkPresentModeKHR &mode) { return mode == VK_PRESENT_MODE_FIFO_KHR; }));
	return std::ranges::any_of(availablePresentModes, [](const VkPresentModeKHR &mode) { return mode == VK_PRESENT_MODE_MAILBOX_KHR; })
			? VK_PRESENT_MODE_MAILBOX_KHR
			: VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRendererLayer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	lm::WindowExtent extent = m_Window->GetExtent();
	return {
		std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
	};
}

uint32_t VulkanRendererLayer::ChooseSwapMinImageCount(const VkSurfaceCapabilitiesKHR &capabilities) const {
	uint32_t minImageCount = std::max(capabilities.minImageCount, 3u);
	if ((capabilities.maxImageCount > 0) && (minImageCount > capabilities.maxImageCount)) {
		minImageCount = capabilities.maxImageCount;
	}
	return minImageCount;
}

std::vector<uint32_t> VulkanRendererLayer::readSpirV(const std::string &filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}
	size_t fileSize = static_cast<size_t>(file.tellg());
	if (fileSize % sizeof(uint32_t) != 0) {
		throw std::runtime_error("Wrong SPIR-V shader format: " + filename);
	}
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
	file.seekg(0);
	file.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(fileSize));
	file.close();
	return buffer;
}

VkShaderModule VulkanRendererLayer::CreateShaderModule(const std::vector<uint32_t> &code) const {
	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size() * sizeof(uint32_t),
		.pCode = code.data(),
	};
	VkShaderModule shaderModule;
	CHECK_VK(vkCreateShaderModule(m_Context.device, &createInfo, nullptr, &shaderModule));
	return shaderModule;
}

void VulkanRendererLayer::RecordCommandBuffer(uint32_t imageIndex) {
	VkCommandBuffer cmd = m_Context.commandBuffers[frameIndex];

	VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	CHECK_VK(vkBeginCommandBuffer(cmd, &beginInfo));
	{
		TransitionImageLayout(
				m_Context.swapchainImages[imageIndex], cmd,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ACCESS_2_NONE,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

		VkClearValue clearColor{ .color = { 0.1f, 0.1f, 0.1f, 1.0f } };
		VkRenderingAttachmentInfo colorAttachment{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = m_Context.swapchainImageViews[imageIndex],
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = clearColor,
		};
		VkRenderingInfo renderingInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea{ .offset = { 0, 0 }, .extent = m_Context.swapchainExtent },
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachment,
		};
		vkCmdBeginRendering(cmd, &renderingInfo);
		{
			VkViewport viewport{
				.x = 0.0f,
				.y = 0.0f,
				.width = static_cast<float>(m_Context.swapchainExtent.width),
				.height = static_cast<float>(m_Context.swapchainExtent.height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			};
			VkRect2D scissor{ .offset = { 0, 0 }, .extent = m_Context.swapchainExtent };
			vkCmdSetViewport(cmd, 0, 1, &viewport);
			vkCmdSetScissor(cmd, 0, 1, &scissor);

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Context.graphicsPipeline);

			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &m_Context.vertexBuffer, &offset);
			vkCmdBindIndexBuffer(cmd, m_Context.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
			auto indexCount = static_cast<uint32_t>(s_Indices.size());
			vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
		}
		vkCmdEndRendering(cmd);

		TransitionImageLayout(
				m_Context.swapchainImages[imageIndex], cmd,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_2_NONE,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
	}
	CHECK_VK(vkEndCommandBuffer(cmd));
}

void VulkanRendererLayer::TransitionImageLayout(
		VkImage image,
		VkCommandBuffer cmd,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkAccessFlags2 srcAccessMask,
		VkAccessFlags2 dstAccessMask,
		VkPipelineStageFlags2 srcStageMask,
		VkPipelineStageFlags2 dstStageMask) {
	VkImageMemoryBarrier2 barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = srcStageMask,
		.srcAccessMask = srcAccessMask,
		.dstStageMask = dstStageMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
		},
	};
	VkDependencyInfo dependencyInfo{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier,
	};
	vkCmdPipelineBarrier2(cmd, &dependencyInfo);
}

uint32_t VulkanRendererLayer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_Context.physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type.");
}

std::pair<VkBuffer, VkDeviceMemory> VulkanRendererLayer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VkBuffer buffer;
	CHECK_VK(vkCreateBuffer(m_Context.device, &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_Context.device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties),
	};
	VkDeviceMemory memory;
	CHECK_VK(vkAllocateMemory(m_Context.device, &allocInfo, nullptr, &memory));

	CHECK_VK(vkBindBufferMemory(m_Context.device, buffer, memory, 0));

	return { buffer, memory };
}

void VulkanRendererLayer::CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size) {
	VkQueue queue = m_Context.graphicsQueue;

	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_Context.commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	VkCommandBuffer cmd;
	CHECK_VK(vkAllocateCommandBuffers(m_Context.device, &allocInfo, &cmd));

	VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	vkBeginCommandBuffer(cmd, &beginInfo);
	{
		VkBufferCopy2 region{
			.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
			.size = size,
		};
		VkCopyBufferInfo2 copyInfo{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
			.srcBuffer = src,
			.dstBuffer = dst,
			.regionCount = 1,
			.pRegions = &region,
		};
		vkCmdCopyBuffer2(cmd, &copyInfo);
	}
	vkEndCommandBuffer(cmd);

	VkFenceCreateInfo fenceInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	VkFence copyFinishedFence;
	CHECK_VK(vkCreateFence(m_Context.device, &fenceInfo, nullptr, &copyFinishedFence));

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd,
	};
	CHECK_VK(vkQueueSubmit(queue, 1, &submitInfo, copyFinishedFence));

	CHECK_VK(vkWaitForFences(m_Context.device, 1, &copyFinishedFence, VK_TRUE, noTimeout));
	vkDestroyFence(m_Context.device, copyFinishedFence, nullptr);
}
