#include "lumen_engine/rhi/vulkan_backend/vulkan_device.hpp"
#include "vulkan_validation.hpp"

#include <algorithm>
#include <ranges>
#include <stdexcept>

namespace lm::rhi {

namespace {

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

	bool foundCombinedGraphicsPresent = false;
	bool foundDedicatedCompute = false;

	for (uint32_t i = 0; i < count; ++i) {
		const auto &family = families[i];
		bool hasGraphics = (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
		bool hasCompute = (family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;

		VkBool32 presentSupport = VK_FALSE;
		CHECK_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport));

		// Graphics fallback
		if (!foundCombinedGraphicsPresent && !indices.graphicsFamily.has_value() && hasGraphics) {
			indices.graphicsFamily = i;
		}

		// Present fallback
		if (!foundCombinedGraphicsPresent && presentSupport && !indices.presentFamily.has_value()) {
			indices.presentFamily = i;
		}

		// Prefered Graphics & Present
		if (!foundCombinedGraphicsPresent && hasGraphics && presentSupport) {
			indices.graphicsFamily = i;
			indices.presentFamily = i;
			foundCombinedGraphicsPresent = true;
		}

		// Compute
		if (!foundDedicatedCompute && hasCompute && !hasGraphics) {
			indices.computeFamily = i;
			foundDedicatedCompute = true;
		} else if (!foundDedicatedCompute && !indices.computeFamily.has_value() && hasCompute) {
			indices.computeFamily = i;
		}
	}

	return indices;
}

} //namespace

void VulkanDevice::PickPhysicalDevice() {
	std::vector<VkPhysicalDevice> devices = EnumeratePhysicalDevices();
	if (devices.empty()) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support.");
	}

	auto it = std::ranges::find_if(devices, [this](VkPhysicalDevice device) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);
		bool supportsVersion = props.apiVersion >= k_VulkanApiVersion;

		bool supportsDeviceExtensions = std::ranges::all_of(k_DeviceExtensions, [&](const char *name) {
			return std::ranges::any_of(EnumerateDeviceExtensions(device), [name](auto &p) {
				return std::strcmp(p.extensionName, name) == 0;
			});
		});

		VkPhysicalDeviceVulkan11Features vulkan11Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
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
		bool supportsFeatures =
				features2.features.samplerAnisotropy &&
				vulkan11Features.shaderDrawParameters &&
				vulkan13Features.dynamicRendering &&
				extendedDynamicStateFeatures.extendedDynamicState;

		return supportsVersion && supportsDeviceExtensions && supportsFeatures && FindQueueFamilies(device, m_Surface).IsComplete();
	});

	if (it == devices.end()) {
		throw std::runtime_error("Failed to find a suitable GPU.");
	}

	m_PhysicalDevice = *it;
	m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, m_Surface);
}

} // namespace lm::rhi
