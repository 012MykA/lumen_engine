#pragma once

#include <lumen_engine/lumen_engine.hpp>
#if defined(LUMEN_PLATFORM_WINDOWS)
	#define VK_USE_PLATFORM_WIN32_KHR
#else
	#error "Unsupported platform"
#endif
#include <vulkan/vulkan.h>

#include <format>
#include <stdexcept>
#include <string>
#include <vector>

class VulkanRendererLayer : public lm::Layer {
public:
	explicit VulkanRendererLayer(lm::Window *window);

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(lm::Event &e) override;
	virtual void OnRender() override;

private:
	const std::vector<const char *> k_RequiredInstanceLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char *> k_RequiredInstanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(LUMEN_PLATFORM_WINDOWS)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
	#error "Unsupported platform"
#endif
	};
	const std::vector<const char *> k_RequiredDeviceExtensions = {};
	const bool k_EnableValidation = true;

	void CreateInstance();
	void CreateDebugMessenger();

	// Helper functions
	std::vector<VkLayerProperties> EnumerateInstanceLayers();
	std::vector<VkExtensionProperties> EnumerateInstanceExtensions();

	std::vector<const char *> FilterSupportedInstanceLayers(const std::vector<const char *> &wantedLayers, bool required);
	std::vector<const char *> FilterSupportedInstanceExtensions(const std::vector<const char *> &wantedExtensions, bool required);

private:
	struct Context {
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
		VkPhysicalDevice gpu = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkQueue queue = VK_NULL_HANDLE;
	};

private:
	lm::Window *m_Window;
	Context m_Context;
};
