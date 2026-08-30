#include "vulkan_renderer_layer.hpp"

#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>
#include <format>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#define CHECK_VK(result) \
	do { \
		if (result != VK_SUCCESS) { \
			std::string msg = \
					std::format("Vulkan error: {} at {}:{}", string_VkResult(result), \
							__FILE__, __LINE__); \
			throw std::runtime_error(msg); \
		} \
	} while (0)

namespace {

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void * /*pUserData*/) {
	auto msg = std::format("Vulkan Validation: type: {}  msg: {}",
			string_VkDebugUtilsMessageTypeFlagsEXT(type),
			pCallbackData->pMessage);
	switch (severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
			LUMEN_CORE_TRACE("{}", msg);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
			LUMEN_CORE_INFO("{}", msg);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
			LUMEN_CORE_WARN("{}", msg);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
			LUMEN_CORE_ERROR("{}", msg);
			break;
		}
		default: {
			LUMEN_CORE_TRACE("{}", msg);
		}
	}
	return VK_FALSE;
}

} //namespace

VulkanRendererLayer::VulkanRendererLayer(lm::Window *window) : Layer("VulkanRendererLayer"), m_Window(window) {}

void VulkanRendererLayer::OnAttach() {
	CreateInstance();
	if (k_EnableValidation) {
		SetupDebugMessenger();
	}
	PickPhysicalDevice();
}

void VulkanRendererLayer::OnDetach() {
	if (m_Context.debugMessenger != VK_NULL_HANDLE) {
		DestroyDebugUtilsMessengerEXT(m_Context.instance, m_Context.debugMessenger, nullptr);
	}
	vkDestroyInstance(m_Context.instance, nullptr);
}

void VulkanRendererLayer::OnEvent(lm::Event &e) {}

void VulkanRendererLayer::OnRender() {}

void VulkanRendererLayer::CreateInstance() {
	constexpr VkApplicationInfo appInfo{
		.pApplicationName = "VulkanRendererLayer",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3,
	};

	std::vector<const char *> wantedLayers = k_RequiredInstanceLayers;
	std::vector<const char *> wantedExtensions = k_RequiredInstanceExtensions;
	if (k_EnableValidation) {
		wantedExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	auto enabledLayers = FilterSupportedInstanceLayers(wantedLayers, /*required=*/true);
	auto enabledExtensions = FilterSupportedInstanceExtensions(wantedExtensions, /*required=*/true);

	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
		.ppEnabledLayerNames = enabledLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
	};

	CHECK_VK(vkCreateInstance(&createInfo, nullptr, &m_Context.instance));

	PrintEnabledInstanceLayers(enabledLayers);
	PrintEnabledInstanceExtensions(enabledExtensions);
}

void VulkanRendererLayer::SetupDebugMessenger() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback,
	};

	CHECK_VK(CreateDebugUtilsMessengerEXT(m_Context.instance, &createInfo, nullptr, &m_Context.debugMessenger));
}

void VulkanRendererLayer::PickPhysicalDevice() {
	std::vector<VkPhysicalDevice> devices = EnumeratePhysicalDevices();
	if (devices.size() == 0) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support.");
	}

	auto deviceIt = std::ranges::find_if(devices, [this](const auto &device) { return IsDeviceSuitable(device); });
	if (deviceIt == devices.end()) {
		throw std::runtime_error{ "Failed to find suitable GPU." };
	}
	m_Context.physicalDevice = *deviceIt;

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(m_Context.physicalDevice, &props);
	LUMEN_CORE_INFO("Using {}", props.deviceName);
}

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
	CHECK_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count,
			available.data()));
	return available;
}

std::vector<VkPhysicalDevice> VulkanRendererLayer::EnumeratePhysicalDevices() const {
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(m_Context.instance, &count, nullptr);
	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(m_Context.instance, &count, devices.data());
	return devices;
}

std::vector<const char *> VulkanRendererLayer::FilterSupportedInstanceLayers(
		const std::vector<const char *> &wantedLayers, bool required) const {
	std::vector<VkLayerProperties> availableLayers = EnumerateInstanceLayers();
	std::vector<const char *> enabledLayers;

	for (const char *wanted : wantedLayers) {
		bool found =
				std::ranges::any_of(availableLayers, [wanted](const auto &prop) {
					return std::strcmp(prop.layerName, wanted) == 0;
				});
		if (found) {
			enabledLayers.push_back(wanted);
		} else if (required) {
			throw std::runtime_error(
					std::format("Required Vulkan layer not supported: {}", wanted));
		} else {
			LUMEN_CORE_WARN("Optional Vulkan layer not supported: {}", wanted);
		}
	}
	return enabledLayers;
}

std::vector<const char *> VulkanRendererLayer::FilterSupportedInstanceExtensions(
		const std::vector<const char *> &wantedExtensions, bool required) const {
	std::vector<VkExtensionProperties> availableExtensions =
			EnumerateInstanceExtensions();
	std::vector<const char *> enabledExtensions;

	for (const char *wanted : wantedExtensions) {
		bool found =
				std::ranges::any_of(availableExtensions, [wanted](const auto &prop) {
					return std::strcmp(prop.extensionName, wanted) == 0;
				});
		if (found) {
			enabledExtensions.push_back(wanted);
		} else if (required) {
			throw std::runtime_error(
					std::format("Required Vulkan extension not supported: {}", wanted));
		} else {
			LUMEN_CORE_WARN("Optional Vulkan extension not supported: {}", wanted);
		}
	}
	return enabledExtensions;
}

QueueFamilyIndices VulkanRendererLayer::FindQueueFamilies(VkPhysicalDevice device) const {
	return QueueFamilyIndices();
}

bool VulkanRendererLayer::IsDeviceSuitable(VkPhysicalDevice device) const {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(device, &props);
	bool supportsVulkan1_3 = props.apiVersion >= VK_API_VERSION_1_4;

	uint32_t queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilies.data());
	bool supportsGraphics = std::ranges::any_of(queueFamilies, [](const VkQueueFamilyProperties &qfp) {
		return !!(qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT);
	});

	VkPhysicalDeviceVulkan11Features vulkan11Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
		.pNext = nullptr,
	};
	VkPhysicalDeviceVulkan13Features vulkan13Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &vulkan11Features,
	};
	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
		.pNext = &vulkan13Features,
	};
	VkPhysicalDeviceFeatures2 features2{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &extendedDynamicStateFeatures,
	};
	vkGetPhysicalDeviceFeatures2(device, &features2);

	bool supportsRequiredFeatures =
			vulkan11Features.shaderDrawParameters &&
			vulkan13Features.dynamicRendering &&
			extendedDynamicStateFeatures.extendedDynamicState;

	return supportsVulkan1_3 && supportsGraphics && supportsRequiredFeatures;
}

void VulkanRendererLayer::PrintEnabledInstanceLayers(const std::vector<const char *> &enabled) const {
	auto availableInstanceLayers = EnumerateInstanceLayers();
	LUMEN_CORE_TRACE("Enabled Instance layers({}/{}):", enabled.size(), availableInstanceLayers.size());
	for (const auto &prop : availableInstanceLayers) {
		bool isEnabled = std::ranges::any_of(enabled, [&prop](const char *name) {
			return std::strcmp(name, prop.layerName) == 0;
		});
		LUMEN_CORE_TRACE("[{}] {}", isEnabled ? '+' : '-', prop.layerName);
	}
	LUMEN_CORE_TRACE("");
}

void VulkanRendererLayer::PrintEnabledInstanceExtensions(const std::vector<const char *> &enabled) const {
	auto availableInstanceExtensions = EnumerateInstanceExtensions();
	LUMEN_CORE_TRACE("Enabled Instance extensions({}/{}):", enabled.size(), availableInstanceExtensions.size());
	for (const auto &prop : availableInstanceExtensions) {
		bool isEnabled = std::ranges::any_of(enabled, [&prop](const char *name) {
			return std::strcmp(name, prop.extensionName) == 0;
		});
		LUMEN_CORE_TRACE("[{}] {}", isEnabled ? '+' : '-', prop.extensionName);
	}
	LUMEN_CORE_TRACE("");
}
