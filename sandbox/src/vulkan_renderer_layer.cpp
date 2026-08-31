#include "vulkan_renderer_layer.hpp"
#ifdef max
	#undef max
#endif

#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <format>
#include <limits>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>

#define CHECK_VK(x) \
	do { \
		VkResult res = x; \
		if (res != VK_SUCCESS) { \
			throw std::runtime_error(std::format("Vulkan error: {} at {}:{}", string_VkResult(res), __FILE__, __LINE__)); \
		} \
	} while (0)

namespace {

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
	auto msg = std::format("Vulkan Validation [{}]: {}", string_VkDebugUtilsMessageTypeFlagsEXT(type), pCallbackData->pMessage);
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
}

void VulkanRendererLayer::OnDetach() {
	vkDestroySwapchainKHR(m_Context.device, m_Context.swapchain, nullptr);
	vkDestroyDevice(m_Context.device, nullptr);
	vkDestroySurfaceKHR(m_Context.instance, m_Context.surface, nullptr);
	if (m_Context.debugMessenger != VK_NULL_HANDLE) {
		DestroyDebugUtilsMessengerEXT(m_Context.instance, m_Context.debugMessenger, nullptr);
	}
	vkDestroyInstance(m_Context.instance, nullptr);
}

void VulkanRendererLayer::OnEvent(lm::Event &e) {
	lm::EventDispatcher dp(e);
	dp.Dispatch<lm::WindowResizeEvent>([this](auto &e) { return OnWindowResize(e.GetExtent()); });
}

void VulkanRendererLayer::OnRender() {}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

bool VulkanRendererLayer::OnWindowResize(lm::WindowExtent extent) {
	LUMEN_CORE_TRACE("VulkanRendererLayer: Framebuffer resized({}, {})", extent.width, extent.height);
	return false;
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
