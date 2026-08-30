#include "vulkan_renderer_layer.hpp"

#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>
#include <format>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#define VK_CHECK(result) \
	do { \
		if (result != VK_SUCCESS) { \
			std::string msg = std::format("Vulkan error: {} at {}:{}", string_VkResult(result), __FILE__, __LINE__); \
			throw std::runtime_error(msg); \
		} \
	} while (0)

VulkanRendererLayer::VulkanRendererLayer(lm::Window *window) : Layer("VulkanRendererLayer"), m_Window(window) {}

void VulkanRendererLayer::OnAttach() {
	CreateInstance();
	if (k_EnableValidation) {
		CreateDebugMessenger();
	}
}

void VulkanRendererLayer::OnDetach() {
	// vkDestroyDebugUtilsMessengerEXT(m_Context.instance, m_Context.debugMessenger, nullptr);
	vkDestroyInstance(m_Context.instance, nullptr);
}

void VulkanRendererLayer::OnEvent(lm::Event &e) {
}

void VulkanRendererLayer::OnRender() {
}

void VulkanRendererLayer::CreateInstance() {
	LUMEN_CORE_TRACE("Initializing Vulkan instance.");

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

	auto availableInstanceExtensions = EnumerateInstanceExtensions();
	LUMEN_CORE_TRACE("Enabled Instsance extensions({}/{}):", enabledExtensions.size(), availableInstanceExtensions.size());
	for (const auto &prop : availableInstanceExtensions) {
		bool isEnabled = std::ranges::any_of(enabledExtensions, [&prop](const char *name) {
			return std::strcmp(name, prop.extensionName) == 0;
		});
		LUMEN_CORE_TRACE("[{}] {}", isEnabled ? '+' : '-', prop.extensionName);
	}

	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
		.ppEnabledLayerNames = enabledLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
	};

	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Context.instance));
}

void VulkanRendererLayer::CreateDebugMessenger() {
}

std::vector<VkLayerProperties> VulkanRendererLayer::EnumerateInstanceLayers() {
	uint32_t count = 0;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&count, nullptr));

	std::vector<VkLayerProperties> available(count);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&count, available.data()));
	return available;
}

std::vector<VkExtensionProperties> VulkanRendererLayer::EnumerateInstanceExtensions() {
	uint32_t count = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));

	std::vector<VkExtensionProperties> available(count);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data()));
	return available;
}

std::vector<const char *> VulkanRendererLayer::FilterSupportedInstanceLayers(
		const std::vector<const char *> &wantedLayers, bool required) {
	std::vector<VkLayerProperties> availableLayers = EnumerateInstanceLayers();
	std::vector<const char *> enabledLayers;

	for (const char *wanted : wantedLayers) {
		bool found = std::ranges::any_of(availableLayers, [wanted](const auto &prop) {
			return std::strcmp(prop.layerName, wanted) == 0;
		});
		if (found) {
			enabledLayers.push_back(wanted);
		} else if (required) {
			throw std::runtime_error(std::format("Required Vulkan layer not supported: {}", wanted));
		} else {
			LUMEN_CORE_WARN("Optional Vulkan layer not supported: {}", wanted);
		}
	}
	return enabledLayers;
}

std::vector<const char *> VulkanRendererLayer::FilterSupportedInstanceExtensions(
		const std::vector<const char *> &wantedExtensions, bool required) {
	std::vector<VkExtensionProperties> availableExtensions = EnumerateInstanceExtensions();
	std::vector<const char *> enabledExtensions;

	for (const char *wanted : wantedExtensions) {
		bool found = std::ranges::any_of(availableExtensions, [wanted](const auto &prop) {
			return std::strcmp(prop.extensionName, wanted) == 0;
		});
		if (found) {
			enabledExtensions.push_back(wanted);
		} else if (required) {
			throw std::runtime_error(std::format("Required Vulkan extension not supported: {}", wanted));
		} else {
			LUMEN_CORE_WARN("Optional Vulkan extension not supported: {}", wanted);
		}
	}
	return enabledExtensions;
}
