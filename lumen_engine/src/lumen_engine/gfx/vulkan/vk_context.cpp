#include "lumen_engine/gfx/vulkan/vk_context.hpp"

#include "lumen_engine/core/log.hpp"
#include "lumen_engine/core/window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <format>
#include <string>

namespace lm {

namespace {

constexpr std::array<const char *, 0> k_InstanceLayers;

constexpr std::array<const char *, 0> k_InstanceExtensions;

// --- Debug Utils Messenger ---
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

} //namespace

VulkanContext::VulkanContext(const Window &window, bool enableValidation) {
	CHECK_VK(volkInitialize());
	if (volkGetInstanceVersion() < VK_API_VERSION_1_3) {
		throw std::runtime_error{ "Vulkan 1.3 is not supported" };
	}
	CreateInstance(enableValidation);
	if (enableValidation) {
		SetupDebugMessenger();
	}
	CreateSurface(window);
}

VulkanContext::~VulkanContext() {
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	if (m_DebugMessenger != VK_NULL_HANDLE) {
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	}
	vkDestroyInstance(m_Instance, nullptr);
	volkFinalize();
}

void VulkanContext::CreateInstance(bool enableValidation) {
	constexpr VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "",
		.applicationVersion = VK_MAKE_VERSION(0, 1, 0),
		.pEngineName = "Lumen Engine",
		.engineVersion = VK_MAKE_VERSION(0, 1, 0),
		.apiVersion = VK_API_VERSION_1_3,
	};

	// --- Extensions ---
	std::vector<const char *> extensions(k_InstanceExtensions.begin(), k_InstanceExtensions.end());

	uint32_t glfwExtCount = 0;
	const char **glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
	extensions.reserve(extensions.size() + glfwExtCount);
	extensions.insert(extensions.end(), glfwExt, glfwExt + glfwExtCount);

	if (enableValidation) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// --- Layers ---
	std::vector<const char *> layers(k_InstanceLayers.begin(), k_InstanceLayers.end());
	if (enableValidation) {
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

void VulkanContext::CreateSurface(const lm::Window &window) {
	CHECK_VK(glfwCreateWindowSurface(m_Instance, window.handle, nullptr, &m_Surface));
}

void VulkanContext::CheckExtensionsSupport(std::span<const char *> extensions) {
	uint32_t count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> available(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());

	auto it = std::ranges::find_if(extensions, [&available](const char *name) {
		return std::ranges::none_of(available, [name](const auto &prop) {
			return std::strcmp(name, prop.extensionName) == 0;
		});
	});
	if (it != extensions.end()) {
		throw std::runtime_error{ "Required instance extension not supported: " + std::string{ *it } };
	}
}

void VulkanContext::CheckLayersSupport(std::span<const char *> layers) {
	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	std::vector<VkLayerProperties> available(count);
	vkEnumerateInstanceLayerProperties(&count, available.data());

	auto it = std::ranges::find_if(layers, [&available](const char *name) {
		return std::ranges::none_of(available, [name](const auto &prop) {
			return std::strcmp(name, prop.layerName) == 0;
		});
	});
	if (it != layers.end()) {
		throw std::runtime_error{ "Required instance layer not supported: " + std::string{ *it } };
	}
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
