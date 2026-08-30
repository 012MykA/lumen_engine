#pragma once

#include <lumen_engine/lumen_engine.hpp>

#if defined(LUMEN_PLATFORM_WINDOWS)
	#define VK_USE_PLATFORM_WIN32_KHR
#else
	#warning "Unsupported platform"
#endif
#include <vulkan/vulkan.h>

#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool IsComplete() const {
		return graphicsFamily.has_value() &&
				presentFamily.has_value() &&
				computeFamily.has_value();
	}
};

class VulkanRendererLayer final : public lm::Layer {
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
	#warning "Unsupported platform"
#endif
	};
	const std::vector<const char *> k_RequiredDeviceExtensions = {};
	const bool k_EnableValidation = true;

	void CreateInstance();
	void SetupDebugMessenger();
	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device) const;

	// Helper functions
	std::vector<VkLayerProperties> EnumerateInstanceLayers() const;
	std::vector<VkExtensionProperties> EnumerateInstanceExtensions() const;
	std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;

	std::vector<const char *> FilterSupportedInstanceLayers(const std::vector<const char *> &wanted, bool required) const;
	std::vector<const char *> FilterSupportedInstanceExtensions(const std::vector<const char *> &wanted, bool required) const;

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;

	// Print
	void PrintEnabledInstanceLayers(const std::vector<const char *> &enabled) const;
	void PrintEnabledInstanceExtensions(const std::vector<const char *> &enabled) const;

private:
	struct Context {
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkQueue queue = VK_NULL_HANDLE;
	};

private:
	lm::Window *m_Window;
	Context m_Context;
};
