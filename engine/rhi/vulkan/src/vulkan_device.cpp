#include "lumen_engine/rhi/vulkan/vulkan_device.hpp"

#include "lumen_engine/io/log.hpp"
#include "vulkan_validation.hpp"

#include <limits>

#ifdef max
	#undef max
#endif

namespace lm::rhi {

namespace {

constexpr auto noTimeout = std::numeric_limits<uint64_t>::max();

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
	// clang-format off
	switch (severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	LUMEN_CORE_TRACE("{}", msg);	break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		LUMEN_CORE_INFO("{}", msg);		break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	LUMEN_CORE_WARN("{}", msg);		break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		LUMEN_CORE_ERROR("{}", msg);	break;
		default: LUMEN_CORE_TRACE("Unknown {}", msg); break;
	}
	// clang-format on
	return VK_FALSE;
}

} // namespace

VulkanDevice::VulkanDevice(void *window) {
	LUMEN_CORE_INFO("[RHI] Initializing Vulkan backend...");

	CreateInstance();
	if (k_EnableValidation) {
		SetupDebugMessenger();
	}
	CreateSurface(window);

	LUMEN_CORE_INFO("[RHI] IDevice initialized successfully");
	LUMEN_CORE_INFO("[RHI] Backend API: Vulkan");
}

VulkanDevice::~VulkanDevice() {
	DestroySurface();
	if (m_DebugMessenger != VK_NULL_HANDLE) {
		DestroyDebugMessenger();
	}
	DestroyInstance();
}

void VulkanDevice::CreateInstance() {
	constexpr VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "No name",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Lumen Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = k_VulkanApiVersion,
	};

	const std::vector<const char *> enabledLayers = FilterSupportedInstanceLayers(k_InstanceLayers, /*required=*/true);
	const std::vector<const char *> enabledExtensions = FilterSupportedInstanceExtensions(k_InstanceExtensions, /*required=*/true);

	const VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
		.ppEnabledLayerNames = enabledLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
	};

	CHECK_VK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
}

void VulkanDevice::SetupDebugMessenger() {
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

void VulkanDevice::CreateSurface(void *window) {
#if defined(LUMEN_PLATFORM_WINDOWS)
	HWND handle = static_cast<HWND>(window);
	const VkWin32SurfaceCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hwnd = handle,
	};
	CHECK_VK(vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface));
#else
	#error Unsupported platform
#endif
}

// -----------------------------------------------

void lm::rhi::VulkanDevice::DestroyDebugMessenger() {
	DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
}

void VulkanDevice::DestroySurface() {
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
}

void VulkanDevice::DestroyInstance() {
	vkDestroyInstance(m_Instance, nullptr);
}

} // namespace lm::rhi
