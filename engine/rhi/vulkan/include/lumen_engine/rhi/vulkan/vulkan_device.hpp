#pragma once

#include "lumen_engine/rhi/device.hpp"

#if defined(LUMEN_PLATFORM_WINDOWS)
	#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

#include <vector>

namespace lm::rhi {

struct VulkanDeviceDesc {
	uint32_t apiVersion = VK_API_VERSION_1_3;
	bool enableValidation = true;
	std::vector<const char *> requiredInstanceLayers = { "VK_LAYER_KHRONOS_validation" };
	std::vector<const char *> requiredInstanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#if defined(LUMEN_PLATFORM_WINDOWS)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	};
	std::vector<const char *> requiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
	};

	uint32_t maxFramesInFlight = 3;
};

class LUMEN_API VulkanDevice final : public IDevice {
public:
	VulkanDevice(void *window, const VulkanDeviceDesc &desc = {});
	~VulkanDevice();

	[[nodiscard]] GraphicsAPI GetAPI() const override { return GraphicsAPI::Vulkan; }

private:
	// --- Init ---
	void CreateInstance(const VulkanDeviceDesc &desc);
	void SetupDebugMessenger();
	void CreateSurface(void *window);

	// --- Query available ---
	std::vector<VkLayerProperties> EnumerateInstanceLayers() const;
	std::vector<VkExtensionProperties> EnumerateInstanceExtensions() const;

	// --- Filter ---
	std::vector<const char *> FilterSupportedInstanceLayers(const std::vector<const char *> &wanted, bool required) const;
	std::vector<const char *> FilterSupportedInstanceExtensions(const std::vector<const char *> &wanted, bool required) const;

private:
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
};

} // namespace lm::rhi
