#include "lumen_engine/vulkan_core/instance.hpp"

#include "lumen_engine/core/log.hpp"
#include "lumen_engine/vulkan_core/check_vk.hpp"

#include <algorithm>
#include <cstring>
#include <format>
#include <stdexcept>
#include <utility>

namespace lmvk {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
	auto msg = std::format("Vulkan Validation [{}]: {}", string_VkDebugUtilsMessageTypeFlagsEXT(type), pCallbackData->pMessage);
	switch (severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			LUMEN_CORE_TRACE("\n\t{}", msg);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LUMEN_CORE_INFO("\n\t{}", msg);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LUMEN_CORE_WARN("\n\t{}", msg);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LUMEN_CORE_ERROR("\n\t{}", msg);
			break;
		default:
			break;
	}
	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *info,
		const VkAllocationCallbacks *alloc, VkDebugUtilsMessengerEXT *messenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	return func ? func(instance, info, alloc, messenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks *alloc) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) {
		func(instance, messenger, alloc);
	}
}

} // namespace

Instance::Instance(const InstanceDesc &desc) {
	CreateVkInstance(desc);
	if (desc.enableValidation) {
		SetupDebugMessenger();
	}
}

Instance::~Instance() {
	if (m_Instance != VK_NULL_HANDLE) {
		if (m_DebugMessenger != VK_NULL_HANDLE) {
			DestroyDebugMessenger();
		}
		vkDestroyInstance(m_Instance, nullptr);
	}
}

Instance::Instance(Instance &&other) noexcept
		: m_Instance(std::exchange(other.m_Instance, VK_NULL_HANDLE)),
		  m_DebugMessenger(std::exchange(other.m_DebugMessenger, VK_NULL_HANDLE)) {}

Instance &Instance::operator=(Instance &&other) noexcept {
	if (this != &other) {
		if (m_DebugMessenger != VK_NULL_HANDLE) {
			DestroyDebugMessenger();
		}
		if (m_Instance != VK_NULL_HANDLE) {
			vkDestroyInstance(m_Instance, nullptr);
		}
		m_Instance = std::exchange(other.m_Instance, VK_NULL_HANDLE);
		m_DebugMessenger = std::exchange(other.m_DebugMessenger, VK_NULL_HANDLE);
	}
	return *this;
}

void Instance::CreateVkInstance(const InstanceDesc &desc) {
	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = desc.appName.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Lumen Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3,
	};

	std::vector<const char *> wantedLayers;
	if (desc.enableValidation) {
		wantedLayers.push_back("VK_LAYER_KHRONOS_validation");
	}

	std::vector<const char *> wantedExtensions = desc.extraExtensions;
	if (desc.enableValidation) {
		wantedExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	auto enabledLayers = FilterSupportedInstanceLayers(wantedLayers, /*required=*/desc.enableValidation);
	auto enabledExtensions = FilterSupportedInstanceExtensions(wantedExtensions, /*required=*/true);

	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
		.ppEnabledLayerNames = enabledLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
	};
	CHECK_VK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
	volkLoadInstance(m_Instance);
}

void Instance::SetupDebugMessenger() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugCallback,
	};
	CHECK_VK(CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger));
}

void Instance::DestroyDebugMessenger() {
	DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
}

std::vector<const char *> Instance::FilterSupportedInstanceLayers(const std::vector<const char *> &wanted, bool required) const {
	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	std::vector<VkLayerProperties> available(count);
	vkEnumerateInstanceLayerProperties(&count, available.data());

	std::vector<const char *> enabled;
	for (const char *name : wanted) {
		bool found = std::ranges::any_of(available, [&](const auto &p) { return std::strcmp(p.layerName, name) == 0; });
		if (found) {
			enabled.push_back(name);
		} else if (required) {
			throw std::runtime_error(std::format("Required Vulkan layer not supported: {}", name));
		}
	}
	return enabled;
}

std::vector<const char *> Instance::FilterSupportedInstanceExtensions(const std::vector<const char *> &wanted, bool required) const {
	uint32_t count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> available(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());

	std::vector<const char *> enabled;
	for (const char *name : wanted) {
		bool found = std::ranges::any_of(available, [&](const auto &p) { return std::strcmp(p.extensionName, name) == 0; });
		if (found) {
			enabled.push_back(name);
		} else if (required) {
			throw std::runtime_error(std::format("Required Vulkan instance extension not supported: {}", name));
		}
	}
	return enabled;
}

} // namespace lmvk
