#include "vulkan_renderer_layer.hpp"

#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>
#include <format>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>

#define CHECK_VK(result) \
	do { \
		if (result != VK_SUCCESS) { \
			throw std::runtime_error(std::format("Vulkan error: {} at {}:{}", string_VkResult(result), __FILE__, __LINE__)); \
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
}

void VulkanRendererLayer::OnDetach() {
	vkDestroyDevice(m_Context.device, nullptr);
	vkDestroySurfaceKHR(m_Context.instance, m_Context.surface, nullptr);
	if (m_Context.debugMessenger != VK_NULL_HANDLE) {
		DestroyDebugUtilsMessengerEXT(m_Context.instance, m_Context.debugMessenger, nullptr);
	}
	vkDestroyInstance(m_Context.instance, nullptr);
}

void VulkanRendererLayer::OnEvent(lm::Event & /*e*/) {}
void VulkanRendererLayer::OnRender() {}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

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
	QueueFamilyIndices indices = FindQueueFamilies(m_Context.physicalDevice);

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

	for (uint32_t i = 0; i < count; ++i) {
		const auto &family = families[i];

		if (!indices.graphicsFamily.has_value() && (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = VK_FALSE;
		CHECK_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Context.surface, &presentSupport));
		if (presentSupport) {
			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.presentFamily = i;
				indices.graphicsFamily = i;
			} else if (!indices.presentFamily.has_value()) {
				indices.presentFamily = i;
			}
		}

		if ((family.queueFlags & VK_QUEUE_COMPUTE_BIT) && !(family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			indices.computeFamily = i;
		} else if (!indices.computeFamily.has_value() && (family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
			indices.computeFamily = i;
		}
	}

	return indices;
}
