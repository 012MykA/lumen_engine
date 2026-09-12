#pragma once

#include "lumen_engine/rhi/device.hpp"

#if defined(LUMEN_PLATFORM_WINDOWS)
	#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

#include <vector>

namespace lm::rhi {

class LUMEN_API VulkanDevice final : public IDevice {
private:
	static constexpr uint32_t k_VulkanApiVersion = VK_API_VERSION_1_3;
	static constexpr bool k_EnableValidation = true;

	const std::vector<const char *> k_InstanceLayers = {
		"VK_LAYER_KHRONOS_validation",
	};

	const std::vector<const char *> k_InstanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(LUMEN_PLATFORM_WINDOWS)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};
	const std::vector<const char *> k_RequiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
	};

	static constexpr uint32_t k_MaxFramesInFlight = 3;

public:
	explicit VulkanDevice(void *window);
	~VulkanDevice();

	GraphicsAPI GetAPI() const override { return GraphicsAPI::Vulkan; }

	void *GetVkInstance() const override { return m_Instance; }
	void *GetVkDebugUtilsMessengerEXT() const override { return m_DebugMessenger; }
	void *GetVkSurfaceKHR() const override { return m_Surface; }

private:
	// --- Init ---
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface(void *window);

	// --- Shutdown ---
	void DestroySurface();
	void DestroyDebugMessenger();
	void DestroyInstance();

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
