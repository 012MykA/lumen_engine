// #include "vulkan_render_layer.hpp"

// #include "cube.hpp"

// #ifdef max
// 	#undef max
// #endif

// #include <imgui.h>
// #include <imgui_impl_glfw.h>
// #include <imgui_impl_vulkan.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

// #include <algorithm>
// #include <array>
// #include <cassert>
// #include <chrono>
// #include <format>
// #include <fstream>
// #include <limits>
// #include <ranges>
// #include <set>
// #include <string>
// #include <thread>
// #include <tuple>

// namespace {

// constexpr auto noTimeout = std::numeric_limits<uint64_t>::max();

// } // namespace

// VulkanRenderLayer::VulkanRenderLayer(lm::Window *window) : Layer("VulkanRenderLayer"), m_Window(window) {}

// void VulkanRenderLayer::OnAttach() {
// 	auto api = lm::rhi::GraphicsAPI::Vulkan;
// 	m_Device = lm::rhi::CreateDevice(api, m_Window->GetNativeHandle());

// 	m_Context.instance = reinterpret_cast<VkInstance>(m_Device->GetVkInstance());
// 	m_Context.debugMessenger = reinterpret_cast<VkDebugUtilsMessengerEXT>(m_Device->GetVkDebugUtilsMessengerEXT());
// 	m_Context.surface = reinterpret_cast<VkSurfaceKHR>(m_Device->GetVkSurfaceKHR());
// 	m_Context.physicalDevice = reinterpret_cast<VkPhysicalDevice>(m_Device->GetVkPhysicalDevice());
// 	m_Context.queueFamilyIndices = *reinterpret_cast<QueueFamilyIndices *>(m_Device->GetQueueFamilyIndices());
// 	m_Context.device = reinterpret_cast<VkDevice>(m_Device->GetVkDevice());
// 	vkGetDeviceQueue(m_Context.device, m_Context.queueFamilyIndices.graphicsFamily.value(), 0, &m_Context.graphicsQueue);
// 	vkGetDeviceQueue(m_Context.device, m_Context.queueFamilyIndices.presentFamily.value(), 0, &m_Context.presentQueue);
// 	vkGetDeviceQueue(m_Context.device, m_Context.queueFamilyIndices.computeFamily.value(), 0, &m_Context.computeQueue);
// 	m_Context.commandPool = reinterpret_cast<VkCommandPool>(m_Device->GetVkCommandPool());

// 	CreateSwapchain();
// 	CreateImageViews();
// 	CreateDescriptorSetLayout();
// 	CreateGraphicsPipeline();

// 	CreateDepthResources();

// 	CreateTextureImage();
// 	m_TextureSampler = m_Device->CreateSampler({});

// 	const lm::rhi::BufferDesc vertexBufferDesc{
// 		.size = sizeof(Cube::vertices[0]) * Cube::vertices.size(),
// 		.usage = lm::rhi::BufferUsage::VertexBuffer,
// 		.memory = lm::rhi::MemoryUsage::DeviceLocal,
// 	};
// 	m_VertexBuffer = m_Device->CreateBuffer(vertexBufferDesc, Cube::vertices.data());

// 	const lm::rhi::BufferDesc indexBufferDesc{
// 		.size = sizeof(Cube::indices[0]) * Cube::indices.size(),
// 		.usage = lm::rhi::BufferUsage::IndexBuffer,
// 		.memory = lm::rhi::MemoryUsage::DeviceLocal,
// 	};
// 	m_IndexBuffer = m_Device->CreateBuffer(indexBufferDesc, Cube::indices.data());

// 	CreateUniformBuffers();
// 	CreateDescriptorPool();
// 	CreateDescriptorSets();

// 	CreateCommandBuffers();
// 	CreateSyncObjects();

// 	// --- ImGui ---
// 	CreateImGuiDescriptorPool();
// 	InitImGui();
// }

// void VulkanRenderLayer::OnDetach() {
// 	CHECK_VK(vkDeviceWaitIdle(m_Context.device));

// 	// --- ImGui ---
// 	ImGui_ImplVulkan_Shutdown();
// 	ImGui_ImplGlfw_Shutdown();
// 	ImGui::DestroyContext();
// 	vkDestroyDescriptorPool(m_Context.device, m_ImGuiDescriptorPool, nullptr);

// 	// --- Main ---
// 	for (size_t i = 0; i < k_MaxFramesInFlight; ++i) {
// 		vkDestroyFence(m_Context.device, m_Context.inFlightFences[i], nullptr);
// 		vkDestroySemaphore(m_Context.device, m_Context.presentCompleteSemaphores[i], nullptr);
// 	}
// 	m_Context.inFlightFences.clear();
// 	m_Context.presentCompleteSemaphores.clear();
// 	for (size_t i = 0; i < m_Context.swapchainImages.size(); ++i) {
// 		vkDestroySemaphore(m_Context.device, m_Context.renderFinishedSemaphores[i], nullptr);
// 	}
// 	m_Context.renderFinishedSemaphores.clear();

// 	vkDestroyDescriptorPool(m_Context.device, m_Context.descriptorPool, nullptr);
// 	m_Context.descriptorSets.clear();

// 	vkDestroyPipelineLayout(m_Context.device, m_Context.graphicsPipelineLayout, nullptr);
// 	vkDestroyDescriptorSetLayout(m_Context.device, m_Context.descriptorSetLayout, nullptr);

// 	CleanupSwapchain();

// 	m_Device.reset();
// }

// void VulkanRenderLayer::OnEvent(lm::Event &e) {
// 	lm::EventDispatcher dp(e);
// 	dp.Dispatch<lm::WindowResizeEvent>([this](auto &e) { return OnWindowResize(e.GetExtent()); });
// }

// void VulkanRenderLayer::OnRender() {
// 	CHECK_VK(vkWaitForFences(m_Context.device, 1, &m_Context.inFlightFences[m_FrameIndex], VK_TRUE, noTimeout));

// 	uint32_t imageIndex;
// 	VkResult result = vkAcquireNextImageKHR(m_Context.device, m_Context.swapchain, noTimeout,
// 			m_Context.presentCompleteSemaphores[m_FrameIndex], VK_NULL_HANDLE, &imageIndex);
// 	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
// 		RecreateSwapchain();
// 		return;
// 	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
// 		CHECK_VK(result);
// 	}

// 	CHECK_VK(vkResetFences(m_Context.device, 1, &m_Context.inFlightFences[m_FrameIndex]));
// 	CHECK_VK(vkResetCommandBuffer(m_Context.commandBuffers[m_FrameIndex], 0));

// 	// ImGui
// 	ImGui_ImplVulkan_NewFrame();
// 	ImGui_ImplGlfw_NewFrame();
// 	ImGui::NewFrame();

// 	ImGui::Render();
// 	// ---

// 	UpdateUniformBuffer();
// 	RecordCommandBuffer(imageIndex);

// 	VkPipelineStageFlags waitDestinationStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
// 	VkSubmitInfo submitInfo{
// 		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
// 		.waitSemaphoreCount = 1,
// 		.pWaitSemaphores = &m_Context.presentCompleteSemaphores[m_FrameIndex],
// 		.pWaitDstStageMask = &waitDestinationStageMask,
// 		.commandBufferCount = 1,
// 		.pCommandBuffers = &m_Context.commandBuffers[m_FrameIndex],
// 		.signalSemaphoreCount = 1,
// 		.pSignalSemaphores = &m_Context.renderFinishedSemaphores[imageIndex],
// 	};
// 	CHECK_VK(vkQueueSubmit(m_Context.graphicsQueue, 1, &submitInfo, m_Context.inFlightFences[m_FrameIndex]));

// 	VkPresentInfoKHR presentInfo{
// 		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
// 		.waitSemaphoreCount = 1,
// 		.pWaitSemaphores = &m_Context.renderFinishedSemaphores[imageIndex],
// 		.swapchainCount = 1,
// 		.pSwapchains = &m_Context.swapchain,
// 		.pImageIndices = &imageIndex,
// 		.pResults = nullptr,
// 	};
// 	result = vkQueuePresentKHR(m_Context.presentQueue, &presentInfo);
// 	if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || m_FramebufferResized) {
// 		m_FramebufferResized = false;
// 		RecreateSwapchain();
// 	} else {
// 		CHECK_VK(result);
// 	}

// 	m_FrameIndex = (m_FrameIndex + 1) % k_MaxFramesInFlight;
// }

// bool VulkanRenderLayer::OnWindowResize(lm::WindowExtent /*extent*/) {
// 	m_FramebufferResized = true;
// 	return false;
// }

// void VulkanRenderLayer::UpdateUniformBuffer() {
// 	static auto startTime = std::chrono::high_resolution_clock::now();
// 	auto currentTime = std::chrono::high_resolution_clock::now();
// 	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

// 	float aspectRatio = static_cast<float>(m_Context.swapchainExtent.width) / m_Context.swapchainExtent.height;
// 	UniformBufferObject ubo{
// 		.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
// 		.view = glm::lookAt(glm::vec3(3.0f), glm::vec3(0.0f), { 0.0f, 0.0f, 1.0f }),
// 	};
// 	ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
// 	ubo.proj[1][1] *= -1;

// 	m_Device->WriteBuffer(m_UniformBuffers[m_FrameIndex], &ubo, sizeof(ubo));
// }

// void VulkanRenderLayer::RecreateSwapchain() {
// 	lm::WindowExtent extent = m_Window->GetExtent();
// 	while (extent.width == 0 || extent.height == 0) {
// 		extent = m_Window->GetExtent();
// 		m_Window->WaitForEvents();
// 	}

// 	CHECK_VK(vkDeviceWaitIdle(m_Context.device));

// 	m_Device->DestroyTexture(m_Context.depthTexture);
// 	CleanupSwapchain();

// 	CreateSwapchain();
// 	CreateImageViews();
// 	CreateDepthResources();
// }

// void VulkanRenderLayer::CleanupSwapchain() {
// 	for (auto imageView : m_Context.swapchainImageViews) {
// 		vkDestroyImageView(m_Context.device, imageView, nullptr);
// 	}
// 	m_Context.swapchainImageViews.clear();
// 	vkDestroySwapchainKHR(m_Context.device, m_Context.swapchain, nullptr);
// 	m_Context.swapchain = VK_NULL_HANDLE;
// }

// void VulkanRenderLayer::CreateSwapchain() {
// 	VkSurfaceCapabilitiesKHR surfaceCapabilities;
// 	CHECK_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context.physicalDevice, m_Context.surface, &surfaceCapabilities));
// 	m_Context.swapchainExtent = ChooseSwapExtent(surfaceCapabilities);
// 	uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

// 	uint32_t formatCount = 0;
// 	CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context.physicalDevice, m_Context.surface, &formatCount, nullptr));
// 	std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
// 	CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context.physicalDevice, m_Context.surface, &formatCount, availableFormats.data()));
// 	m_Context.swapchainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

// 	uint32_t presentModeCount = 0;
// 	CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context.physicalDevice, m_Context.surface, &presentModeCount, nullptr));
// 	std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
// 	CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context.physicalDevice, m_Context.surface, &presentModeCount, availablePresentModes.data()));
// 	VkPresentModeKHR presentMode = ChooseSwapPresentMode(availablePresentModes);

// 	const QueueFamilyIndices &indices = m_Context.queueFamilyIndices;
// 	std::array<uint32_t, 2> queueFamilyIndices = {
// 		indices.graphicsFamily.value(),
// 		indices.presentFamily.value(),
// 	};
// 	bool sameFamily = indices.graphicsFamily.value() == indices.presentFamily.value();

// 	VkSwapchainCreateInfoKHR createInfo{
// 		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
// 		.surface = m_Context.surface,
// 		.minImageCount = minImageCount,
// 		.imageFormat = m_Context.swapchainSurfaceFormat.format,
// 		.imageColorSpace = m_Context.swapchainSurfaceFormat.colorSpace,
// 		.imageExtent = m_Context.swapchainExtent,
// 		.imageArrayLayers = 1,
// 		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
// 		.imageSharingMode = sameFamily ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
// 		.queueFamilyIndexCount = sameFamily ? 0 : static_cast<uint32_t>(queueFamilyIndices.size()),
// 		.pQueueFamilyIndices = sameFamily ? nullptr : queueFamilyIndices.data(),
// 		.preTransform = surfaceCapabilities.currentTransform,
// 		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
// 		.presentMode = presentMode,
// 		.clipped = VK_TRUE,
// 		.oldSwapchain = m_Context.swapchain,
// 	};
// 	CHECK_VK(vkCreateSwapchainKHR(m_Context.device, &createInfo, nullptr, &m_Context.swapchain));

// 	uint32_t swapchainImageCount;
// 	vkGetSwapchainImagesKHR(m_Context.device, m_Context.swapchain, &swapchainImageCount, nullptr);
// 	m_Context.swapchainImages.resize(swapchainImageCount);
// 	vkGetSwapchainImagesKHR(m_Context.device, m_Context.swapchain, &swapchainImageCount, m_Context.swapchainImages.data());
// }

// void VulkanRenderLayer::CreateImageViews() {
// 	assert(m_Context.swapchainImageViews.empty());

// 	size_t imageCount = m_Context.swapchainImages.size();
// 	m_Context.swapchainImageViews.resize(imageCount);
// 	for (size_t i = 0; i < imageCount; ++i) {
// 		VkImageViewCreateInfo createInfo{
// 			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
// 			.image = m_Context.swapchainImages[i],
// 			.viewType = VK_IMAGE_VIEW_TYPE_2D,
// 			.format = m_Context.swapchainSurfaceFormat.format,
// 			.subresourceRange{
// 					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
// 					.baseMipLevel = 0,
// 					.levelCount = 1,
// 					.baseArrayLayer = 0,
// 					.layerCount = 1,
// 			},
// 		};
// 		CHECK_VK(vkCreateImageView(m_Context.device, &createInfo, nullptr, &m_Context.swapchainImageViews[i]));
// 	}
// }

// void VulkanRenderLayer::CreateGraphicsPipeline() {
// 	// Pipeline layout
// 	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
// 		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
// 		.setLayoutCount = 1,
// 		.pSetLayouts = &m_Context.descriptorSetLayout,
// 		.pushConstantRangeCount = 0,
// 	};
// 	CHECK_VK(vkCreatePipelineLayout(m_Context.device, &pipelineLayoutInfo, nullptr, &m_Context.graphicsPipelineLayout));

// 	// Shader
// 	auto code = readSpirV("shaders/bin/shader.spv");
// 	m_Shader = m_Device->CreateShader(code);

// 	lm::rhi::Format swapchainFormat;
// 	switch (m_Context.swapchainSurfaceFormat.format) {
// 		case VK_FORMAT_B8G8R8A8_SRGB: {
// 			swapchainFormat = lm::rhi::Format::BGRA8_SRGB;
// 			break;
// 		}
// 		case VK_FORMAT_B8G8R8A8_UNORM: {
// 			swapchainFormat = lm::rhi::Format::BGRA8_UNORM;
// 			break;
// 		}
// 		default:
// 			throw std::runtime_error("CreateGraphicsPipeline(): unsupported swapchain surface format");
// 	}

// 	lm::rhi::GraphicsPipelineDesc desc{
// 		.shaderStages = {
// 				{ lm::rhi::ShaderStage::Vertex, m_Shader, "vertMain" },
// 				{ lm::rhi::ShaderStage::Fragment, m_Shader, "fragMain" },
// 		},
// 		.vertexBindings = { Vertex::GetVertexBinding() },

// 		.topology = lm::rhi::PrimitiveTopology::TriangleList,
// 		.cullMode = lm::rhi::CullMode::Back,
// 		.frontFace = lm::rhi::FrontFace::CounterClockwise,

// 		.depthStencil{
// 				.depthTestEnable = true,
// 				.depthWriteEnable = true,
// 				.depthCompareOp = lm::rhi::CompareOp::Less,
// 		},

// 		.colorAttachmentFormats = { swapchainFormat },
// 		.depthAttachmentFormat = lm::rhi::Format::D32_SFLOAT, // m_Context.depthFormat

// 		.pipelineLayout = m_Context.graphicsPipelineLayout,
// 	};
// 	m_GraphicsPipeline = m_Device->CreatePipeline(desc);

// 	m_Device->DestroyShader(m_Shader);
// }

// void VulkanRenderLayer::CreateDepthResources() {
// 	lm::rhi::TextureDesc desc{
// 		.width = m_Context.swapchainExtent.width,
// 		.height = m_Context.swapchainExtent.height,
// 		.format = lm::rhi::Format::D32_SFLOAT, // m_Context.depthFormat
// 		.usage = lm::rhi::TextureUsage::DepthStencilAttachment
// 	};
// 	m_Context.depthTexture = m_Device->CreateTexture(desc);
// }

// void VulkanRenderLayer::CreateCommandBuffers() {
// 	VkCommandBufferAllocateInfo allocInfo{
// 		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
// 		.commandPool = m_Context.commandPool,
// 		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
// 		.commandBufferCount = k_MaxFramesInFlight,
// 	};
// 	m_Context.commandBuffers.resize(k_MaxFramesInFlight);
// 	CHECK_VK(vkAllocateCommandBuffers(m_Context.device, &allocInfo, m_Context.commandBuffers.data()));
// }

// void VulkanRenderLayer::CreateSyncObjects() {
// 	assert(m_Context.presentCompleteSemaphores.empty() &&
// 			m_Context.renderFinishedSemaphores.empty() &&
// 			m_Context.inFlightFences.empty());

// 	VkSemaphoreCreateInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

// 	const size_t swapchainImageCount = m_Context.swapchainImages.size();
// 	m_Context.renderFinishedSemaphores.resize(swapchainImageCount);
// 	for (size_t i = 0; i < swapchainImageCount; ++i) {
// 		CHECK_VK(vkCreateSemaphore(m_Context.device, &semaphoreInfo, nullptr, &m_Context.renderFinishedSemaphores[i]));
// 	}

// 	m_Context.presentCompleteSemaphores.resize(k_MaxFramesInFlight);
// 	m_Context.inFlightFences.resize(k_MaxFramesInFlight);
// 	for (size_t i = 0; i < k_MaxFramesInFlight; ++i) {
// 		CHECK_VK(vkCreateSemaphore(m_Context.device, &semaphoreInfo, nullptr, &m_Context.presentCompleteSemaphores[i]));

// 		VkFenceCreateInfo fenceInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };
// 		CHECK_VK(vkCreateFence(m_Context.device, &fenceInfo, nullptr, &m_Context.inFlightFences[i]));
// 	}
// }

// void VulkanRenderLayer::CreateImGuiDescriptorPool() {
// 	VkDescriptorPoolSize poolSizes[] = {
// 		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
// 		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
// 	};

// 	VkDescriptorPoolCreateInfo createInfo = {
// 		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
// 		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
// 		.maxSets = 1000 * IM_ARRAYSIZE(poolSizes),
// 		.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes)),
// 		.pPoolSizes = poolSizes
// 	};

// 	CHECK_VK(vkCreateDescriptorPool(m_Context.device, &createInfo, nullptr, &m_ImGuiDescriptorPool));
// }

// void VulkanRenderLayer::InitImGui() {
// 	ImGui::CreateContext();

// 	ImGuiIO &io = ImGui::GetIO();
// 	io.DisplaySize.x = (float)m_Context.swapchainExtent.width;
// 	io.DisplaySize.y = (float)m_Context.swapchainExtent.height;

// 	ImGui::GetStyle().FontScaleMain = 1.5f;

// 	ImGui::StyleColorsDark();

// 	bool installGlfwCallbacks = true;
// 	GLFWwindow *window = reinterpret_cast<GLFWwindow *>(m_Window->GetGLFWwindow());
// 	ImGui_ImplGlfw_InitForVulkan(window, installGlfwCallbacks);

// 	VkSurfaceCapabilitiesKHR caps;
// 	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context.physicalDevice, m_Context.surface, &caps);

// 	ImGui_ImplVulkan_InitInfo initInfo{
// 		.ApiVersion = VK_VERSION_1_3,
// 		.Instance = m_Context.instance,
// 		.PhysicalDevice = m_Context.physicalDevice,
// 		.Device = m_Context.device,
// 		.QueueFamily = m_Context.queueFamilyIndices.graphicsFamily.value(),
// 		.Queue = m_Context.graphicsQueue,
// 		.DescriptorPool = m_ImGuiDescriptorPool,
// 		.MinImageCount = caps.minImageCount,
// 		.ImageCount = static_cast<uint32_t>(m_Context.swapchainImages.size()),
// 		.PipelineCache = nullptr,
// 		.PipelineInfoMain = {
// 				.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
// 				.PipelineRenderingCreateInfo = {
// 						.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
// 						.viewMask = 0,
// 						.colorAttachmentCount = 1,
// 						.pColorAttachmentFormats = &m_Context.swapchainSurfaceFormat.format,
// 						.depthAttachmentFormat = m_Context.depthFormat,
// 						.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
// 				},
// 		},
// 		.UseDynamicRendering = true,
// 		.Allocator = nullptr,
// 		.CheckVkResultFn = [](VkResult res) { CHECK_VK(res); },
// 	};
// 	ImGui_ImplVulkan_Init(&initInfo);
// }

// void VulkanRenderLayer::CreateTextureImage() {
// 	int texWidth, texHeight, texChannels;
// 	const int desiredChannels = STBI_rgb_alpha;
// 	stbi_uc *pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, desiredChannels);
// 	if (!pixels) {
// 		throw std::runtime_error("failed to load image");
// 	}

// 	lm::rhi::TextureDesc desc{
// 		.width = static_cast<uint32_t>(texWidth),
// 		.height = static_cast<uint32_t>(texHeight),
// 		.mipLevels = lm::rhi::TextureDesc::k_AutoCalcMipLevels,
// 		.generateMipmaps = true,
// 		.format = lm::rhi::Format::RGBA8_SRGB,
// 	};
// 	m_Texture = m_Device->CreateTexture(desc, pixels);

// 	stbi_image_free(pixels);
// }

// void VulkanRenderLayer::CreateUniformBuffers() {
// 	m_UniformBuffers.resize(k_MaxFramesInFlight);
// 	for (size_t i = 0; i < k_MaxFramesInFlight; ++i) {
// 		lm::rhi::BufferDesc desc{
// 			.size = sizeof(UniformBufferObject),
// 			.usage = lm::rhi::BufferUsage::UniformBuffer,
// 			.memory = lm::rhi::MemoryUsage::HostVisible,
// 		};
// 		m_UniformBuffers[i] = m_Device->CreateBuffer(desc);
// 	}
// }

// void VulkanRenderLayer::CreateDescriptorPool() {
// 	std::array<VkDescriptorPoolSize, 2> poolSizes{
// 		VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = k_MaxFramesInFlight },
// 		VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = k_MaxFramesInFlight },
// 	};

// 	VkDescriptorPoolCreateInfo createInfo{
// 		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
// 		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
// 		.maxSets = k_MaxFramesInFlight,
// 		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
// 		.pPoolSizes = poolSizes.data(),
// 	};
// 	CHECK_VK(vkCreateDescriptorPool(m_Context.device, &createInfo, nullptr, &m_Context.descriptorPool));
// }

// void VulkanRenderLayer::CreateDescriptorSets() {
// 	std::vector<VkDescriptorSetLayout> layouts(k_MaxFramesInFlight, m_Context.descriptorSetLayout);

// 	VkDescriptorSetAllocateInfo allocInfo{
// 		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
// 		.descriptorPool = m_Context.descriptorPool,
// 		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
// 		.pSetLayouts = layouts.data(),
// 	};
// 	m_Context.descriptorSets.resize(layouts.size());
// 	CHECK_VK(vkAllocateDescriptorSets(m_Context.device, &allocInfo, m_Context.descriptorSets.data()));

// 	std::vector<VkWriteDescriptorSet> writes;
// 	writes.reserve(k_MaxFramesInFlight * 2);

// 	std::vector<VkDescriptorBufferInfo> bufferInfos(k_MaxFramesInFlight);
// 	std::vector<VkDescriptorImageInfo> imageInfos(k_MaxFramesInFlight);

// 	for (size_t i = 0; i < k_MaxFramesInFlight; ++i) {
// 		// Binding 0
// 		VkBuffer uniformBuffer = reinterpret_cast<VkBuffer>(m_Device->GetVkBuffer(m_UniformBuffers[i]));
// 		bufferInfos[i] = VkDescriptorBufferInfo{
// 			.buffer = uniformBuffer,
// 			.offset = 0,
// 			.range = sizeof(UniformBufferObject),
// 		};

// 		VkWriteDescriptorSet bufferWrite{
// 			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
// 			.dstSet = m_Context.descriptorSets[i],
// 			.dstBinding = 0,
// 			.dstArrayElement = 0,
// 			.descriptorCount = 1,
// 			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
// 			.pImageInfo = nullptr,
// 			.pBufferInfo = &bufferInfos[i],
// 		};
// 		writes.push_back(bufferWrite);

// 		// Binding 1
// 		imageInfos[i] = VkDescriptorImageInfo{
// 			.sampler = reinterpret_cast<VkSampler>(m_Device->GetVkSampler(m_TextureSampler)),
// 			.imageView = reinterpret_cast<VkImageView>(m_Device->GetVkImageView(m_Texture)),
// 			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
// 		};

// 		VkWriteDescriptorSet imageWrite{
// 			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
// 			.dstSet = m_Context.descriptorSets[i],
// 			.dstBinding = 1,
// 			.dstArrayElement = 0,
// 			.descriptorCount = 1,
// 			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
// 			.pImageInfo = &imageInfos[i],
// 		};
// 		writes.push_back(imageWrite);
// 	}
// 	vkUpdateDescriptorSets(m_Context.device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
// }

// void VulkanRenderLayer::CreateDescriptorSetLayout() {
// 	std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
// 		VkDescriptorSetLayoutBinding{
// 				.binding = 0,
// 				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
// 				.descriptorCount = 1,
// 				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
// 		},
// 		VkDescriptorSetLayoutBinding{
// 				.binding = 1,
// 				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
// 				.descriptorCount = 1,
// 				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
// 		}
// 	};
// 	VkDescriptorSetLayoutCreateInfo layoutInfo{
// 		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
// 		.bindingCount = static_cast<uint32_t>(bindings.size()),
// 		.pBindings = bindings.data(),
// 	};
// 	CHECK_VK(vkCreateDescriptorSetLayout(m_Context.device, &layoutInfo, nullptr, &m_Context.descriptorSetLayout));
// }

// VkSurfaceFormatKHR VulkanRenderLayer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const {
// 	const auto formatIt = std::ranges::find_if(availableFormats, [](const auto &fmt) {
// 		return fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
// 	});
// 	return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
// }

// VkPresentModeKHR VulkanRenderLayer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const {
// 	assert(std::ranges::any_of(availablePresentModes, [](const VkPresentModeKHR &mode) { return mode == VK_PRESENT_MODE_FIFO_KHR; }));
// 	return std::ranges::any_of(availablePresentModes, [](const VkPresentModeKHR &mode) { return mode == VK_PRESENT_MODE_MAILBOX_KHR; })
// 			? VK_PRESENT_MODE_MAILBOX_KHR
// 			: VK_PRESENT_MODE_FIFO_KHR;
// }

// VkExtent2D VulkanRenderLayer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
// 	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
// 		return capabilities.currentExtent;
// 	}
// 	lm::WindowExtent extent = m_Window->GetExtent();
// 	return {
// 		std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
// 		std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
// 	};
// }

// uint32_t VulkanRenderLayer::ChooseSwapMinImageCount(const VkSurfaceCapabilitiesKHR &capabilities) const {
// 	uint32_t minImageCount = std::max(capabilities.minImageCount, 3u);
// 	if ((capabilities.maxImageCount > 0) && (minImageCount > capabilities.maxImageCount)) {
// 		minImageCount = capabilities.maxImageCount;
// 	}
// 	return minImageCount;
// }

// std::vector<uint32_t> VulkanRenderLayer::readSpirV(const std::string &filename) {
// 	std::ifstream file(filename, std::ios::ate | std::ios::binary);
// 	if (!file.is_open()) {
// 		throw std::runtime_error("Failed to open file: " + filename);
// 	}
// 	size_t fileSize = static_cast<size_t>(file.tellg());
// 	if (fileSize % sizeof(uint32_t) != 0) {
// 		throw std::runtime_error("Wrong SPIR-V shader format: " + filename);
// 	}
// 	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
// 	file.seekg(0);
// 	file.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(fileSize));
// 	file.close();
// 	return buffer;
// }

// void VulkanRenderLayer::RecordCommandBuffer(uint32_t imageIndex) {
// 	VkCommandBuffer cmd = m_Context.commandBuffers[m_FrameIndex];

// 	VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
// 	CHECK_VK(vkBeginCommandBuffer(cmd, &beginInfo));
// 	{
// 		TransitionImageLayout(
// 				cmd,
// 				m_Context.swapchainImages[imageIndex],
// 				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
// 				VK_ACCESS_2_NONE,
// 				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
// 				VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
// 				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
// 				VK_IMAGE_ASPECT_COLOR_BIT);

// 		TransitionImageLayout(
// 				cmd,
// 				reinterpret_cast<VkImage>(m_Device->GetVkImage(m_Context.depthTexture)),
// 				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
// 				VK_ACCESS_2_NONE,
// 				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
// 				VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
// 				VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
// 				VK_IMAGE_ASPECT_DEPTH_BIT);

// 		VkClearValue clearColor{ .color = { 0.1f, 0.1f, 0.1f, 1.0f } };
// 		VkClearValue depthColor{ .depthStencil = { 1.0f, 0 } };

// 		VkRenderingAttachmentInfo colorAttachment{
// 			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
// 			.imageView = m_Context.swapchainImageViews[imageIndex],
// 			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
// 			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
// 			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
// 			.clearValue = clearColor,
// 		};

// 		VkRenderingAttachmentInfo depthAttachment{
// 			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
// 			.imageView = reinterpret_cast<VkImageView>(m_Device->GetVkImageView(m_Context.depthTexture)),
// 			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
// 			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
// 			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
// 			.clearValue = depthColor,
// 		};

// 		VkRenderingInfo renderingInfo{
// 			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
// 			.renderArea{ .offset = { 0, 0 }, .extent = m_Context.swapchainExtent },
// 			.layerCount = 1,
// 			.colorAttachmentCount = 1,
// 			.pColorAttachments = &colorAttachment,
// 			.pDepthAttachment = &depthAttachment,
// 		};
// 		vkCmdBeginRendering(cmd, &renderingInfo);
// 		{
// 			VkViewport viewport{
// 				.x = 0.0f,
// 				.y = 0.0f,
// 				.width = static_cast<float>(m_Context.swapchainExtent.width),
// 				.height = static_cast<float>(m_Context.swapchainExtent.height),
// 				.minDepth = 0.0f,
// 				.maxDepth = 1.0f
// 			};
// 			VkRect2D scissor{ .offset = { 0, 0 }, .extent = m_Context.swapchainExtent };
// 			vkCmdSetViewport(cmd, 0, 1, &viewport);
// 			vkCmdSetScissor(cmd, 0, 1, &scissor);

// 			VkPipeline pipeline = reinterpret_cast<VkPipeline>(m_Device->GetVkPipeline(m_GraphicsPipeline));
// 			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

// 			VkDeviceSize offset = 0;
// 			std::array vertexBuffers = { reinterpret_cast<VkBuffer>(m_Device->GetVkBuffer(m_VertexBuffer)) };
// 			vkCmdBindVertexBuffers(cmd, 0, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), &offset);

// 			VkBuffer indexBuffer = reinterpret_cast<VkBuffer>(m_Device->GetVkBuffer(m_IndexBuffer));
// 			vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

// 			std::array descriptorSets = { m_Context.descriptorSets[m_FrameIndex] };
// 			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
// 					m_Context.graphicsPipelineLayout, 0,
// 					static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
// 					0, nullptr);

// 			vkCmdDrawIndexed(cmd, static_cast<uint32_t>(Cube::indices.size()), 1, 0, 0, 0);

// 			// ImGui
// 			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
// 			// ---
// 		}
// 		vkCmdEndRendering(cmd);

// 		TransitionImageLayout(
// 				cmd,
// 				m_Context.swapchainImages[imageIndex],
// 				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
// 				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
// 				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
// 				VK_ACCESS_2_NONE,
// 				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
// 				VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
// 				VK_IMAGE_ASPECT_COLOR_BIT);
// 	}
// 	CHECK_VK(vkEndCommandBuffer(cmd));
// }

// void VulkanRenderLayer::TransitionImageLayout(
// 		VkCommandBuffer cmd,
// 		VkImage image,
// 		VkImageLayout oldLayout,
// 		VkImageLayout newLayout,
// 		VkAccessFlags2 srcAccessMask,
// 		VkAccessFlags2 dstAccessMask,
// 		VkPipelineStageFlags2 srcStageMask,
// 		VkPipelineStageFlags2 dstStageMask,
// 		VkImageAspectFlags aspectFlags) {
// 	VkImageMemoryBarrier2 barrier{
// 		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
// 		.srcStageMask = srcStageMask,
// 		.srcAccessMask = srcAccessMask,
// 		.dstStageMask = dstStageMask,
// 		.dstAccessMask = dstAccessMask,
// 		.oldLayout = oldLayout,
// 		.newLayout = newLayout,
// 		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
// 		.image = image,
// 		.subresourceRange{
// 				.aspectMask = aspectFlags,
// 				.baseMipLevel = 0,
// 				.levelCount = 1,
// 				.baseArrayLayer = 0,
// 				.layerCount = 1,
// 		},
// 	};
// 	VkDependencyInfo dependencyInfo{
// 		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
// 		.imageMemoryBarrierCount = 1,
// 		.pImageMemoryBarriers = &barrier,
// 	};
// 	vkCmdPipelineBarrier2(cmd, &dependencyInfo);
// }
