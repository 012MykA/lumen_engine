#include "lumen_engine/gfx/vulkan/vk_context.hpp"

#include "lumen_engine/core/log.hpp"
#include "lumen_engine/core/window.hpp"
#include "lumen_engine/gfx/vulkan/device_feature_chain.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <format>
#include <set>
#include <string>
#include <vector>

namespace lm {

namespace {

constexpr bool k_ValidationEnabled = true;

constexpr uint32_t k_VulkanApiVersion = VK_API_VERSION_1_3;

constexpr std::array<const char *, 0> k_InstanceLayers;

constexpr std::array<const char *, 0> k_InstanceExtensions;

constexpr std::array k_DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
};

void FillRequiredFeatures(DeviceFeatureChain &chain) {
	chain.features2.features.samplerAnisotropy = VK_TRUE;
	chain.vk13.dynamicRendering = VK_TRUE;
	chain.vk13.synchronization2 = VK_TRUE;
}

// --- Debug Utils Messenger ---
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

} //namespace

VulkanContext::VulkanContext(const Window &window) : m_Window{ window } {
	CHECK_VK(volkInitialize());
	if (volkGetInstanceVersion() < k_VulkanApiVersion) {
		throw std::runtime_error{ "Vulkan 1.3 is not supported" };
	}
	CreateInstance();
	if constexpr (k_ValidationEnabled) {
		SetupDebugMessenger();
	}
	CreateSurface();
	PickPhysicalDevice();
	CreateDevice();
	CreateAllocator();
}

VulkanContext::~VulkanContext() {
	if (vkDeviceWaitIdle(m_Device) != VK_SUCCESS) {
		LUMEN_CORE_ERROR("failed to vkDeviceWaitIdle in VulkanContext destructor");
	}

	vmaDestroyAllocator(m_Allocator);
	vkDestroyDevice(m_Device, nullptr);
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	if (m_DebugMessenger != VK_NULL_HANDLE) {
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	}
	vkDestroyInstance(m_Instance, nullptr);
	volkFinalize();
}

void VulkanContext::CreateInstance() {
	constexpr VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "",
		.applicationVersion = VK_MAKE_VERSION(0, 1, 0),
		.pEngineName = "Lumen Engine",
		.engineVersion = VK_MAKE_VERSION(0, 1, 0),
		.apiVersion = k_VulkanApiVersion,
	};

	// --- Extensions ---
	std::vector<const char *> extensions(k_InstanceExtensions.begin(), k_InstanceExtensions.end());

	uint32_t glfwExtCount = 0;
	const char **glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
	extensions.reserve(extensions.size() + glfwExtCount);
	extensions.insert(extensions.end(), glfwExt, glfwExt + glfwExtCount);

	if constexpr (k_ValidationEnabled) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// --- Layers ---
	std::vector<const char *> layers(k_InstanceLayers.begin(), k_InstanceLayers.end());
	if constexpr (k_ValidationEnabled) {
		layers.push_back("VK_LAYER_KHRONOS_validation");
	}

	CheckExtensionsSupport(extensions);
	CheckLayersSupport(layers);
	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
	};

	CHECK_VK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
	volkLoadInstance(m_Instance);
}

void VulkanContext::SetupDebugMessenger() {
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
		.pfnUserCallback = DebugCallback,
	};
	CHECK_VK(CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger));
}

void VulkanContext::CreateSurface() {
	CHECK_VK(glfwCreateWindowSurface(m_Instance, m_Window.handle, nullptr, &m_Surface));
}

void VulkanContext::PickPhysicalDevice() {
	uint32_t count = 0;
	CHECK_VK(vkEnumeratePhysicalDevices(m_Instance, &count, nullptr));
	std::vector<VkPhysicalDevice> devices(count);
	CHECK_VK(vkEnumeratePhysicalDevices(m_Instance, &count, devices.data()));
	if (devices.empty()) {
		throw std::runtime_error{ "Failed to find Graphics Device with Vulkan support" };
	}

	const auto it = std::ranges::find_if(devices, [this](VkPhysicalDevice d) { return IsDeviceSuitable(d); });
	if (it == devices.end()) {
		throw std::runtime_error{ "Failed to find a suitable Graphics Device" };
	}
	m_PhysicalDevice = *it;
	m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, m_Surface);
}

void VulkanContext::CreateDevice() {
	const QueueFamilyIndices &indices = m_QueueFamilyIndices;

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

	DeviceFeatureChain features;
	FillRequiredFeatures(features);

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features.features2,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledExtensionCount = static_cast<uint32_t>(k_DeviceExtensions.size()),
		.ppEnabledExtensionNames = k_DeviceExtensions.data(),
		.pEnabledFeatures = nullptr,
	};
	CHECK_VK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));
	volkLoadDevice(m_Device);

	vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
	vkGetDeviceQueue(m_Device, indices.computeFamily.value(), 0, &m_ComputeQueue);
}

void VulkanContext::CreateAllocator() {
	VmaVulkanFunctions vulkanFuncs{
		.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
		.vkGetDeviceProcAddr = vkGetDeviceProcAddr,
	};
	VmaAllocatorCreateInfo createInfo{
		.physicalDevice = m_PhysicalDevice,
		.device = m_Device,
		.pVulkanFunctions = &vulkanFuncs,
		.instance = m_Instance,
		.vulkanApiVersion = k_VulkanApiVersion,
	};
	CHECK_VK(vmaCreateAllocator(&createInfo, &m_Allocator));
}

void VulkanContext::CheckExtensionsSupport(std::span<const char *> extensions) const {
	uint32_t count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> available(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());

	const auto it = std::ranges::find_if(extensions, [&available](const char *name) {
		return std::ranges::none_of(available, [name](const auto &prop) {
			return std::strcmp(name, prop.extensionName) == 0;
		});
	});
	if (it != extensions.end()) {
		throw std::runtime_error{ "Required instance extension not supported: " + std::string{ *it } };
	}
}

void VulkanContext::CheckLayersSupport(std::span<const char *> layers) const {
	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	std::vector<VkLayerProperties> available(count);
	vkEnumerateInstanceLayerProperties(&count, available.data());

	const auto it = std::ranges::find_if(layers, [&available](const char *name) {
		return std::ranges::none_of(available, [name](const auto &prop) {
			return std::strcmp(name, prop.layerName) == 0;
		});
	});
	if (it != layers.end()) {
		throw std::runtime_error{ "Required instance layer not supported: " + std::string{ *it } };
	}
}

bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device) const {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(device, &props);
	bool supportsVersion = props.apiVersion >= k_VulkanApiVersion;

	DeviceFeatureChain chain;
	chain.QueryFor(device);
	bool supportsFeatures = chain.features2.features.samplerAnisotropy &&
			chain.vk13.dynamicRendering &&
			chain.extendedDynamicState.extendedDynamicState;

	return supportsVersion && supportsFeatures && FindQueueFamilies(device, m_Surface).IsComplete();
}

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
	std::string msg = std::format("\n\tVulkan Validation [{}]: {}", string_VkDebugUtilsMessageTypeFlagsEXT(type), pCallbackData->pMessage);
	switch (severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LUMEN_CORE_INFO("{}", msg);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LUMEN_CORE_WARN("{}", msg);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LUMEN_CORE_ERROR("{}", msg);
			break;
		default:
			LUMEN_CORE_TRACE("{}", msg);
			break;
	}
	return VK_FALSE;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) {
		func(instance, debugMessenger, pAllocator);
	}
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	return func ? func(instance, pCreateInfo, pAllocator, pDebugMessenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

} //namespace

} // namespace lm
