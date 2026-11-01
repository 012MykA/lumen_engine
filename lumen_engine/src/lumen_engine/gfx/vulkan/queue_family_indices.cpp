#include "lumen_engine/gfx/vulkan/queue_family_indices.hpp"

#include <vector>

namespace lm {

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

	std::vector<VkBool32> canPresent(count, VK_FALSE);
	for (uint32_t i = 0; i < count; ++i) {
		if (families[i].queueCount > 0) {
			CHECK_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &canPresent[i]));
		}
	}

	auto hasFlag = [&](uint32_t i, VkQueueFlags flag) {
		return families[i].queueCount > 0 && (families[i].queueFlags & flag);
	};

	QueueFamilyIndices indices;

	// Graphics + Present
	for (uint32_t i = 0; i < count; ++i) {
		if (hasFlag(i, VK_QUEUE_GRAPHICS_BIT) && canPresent[i]) {
			indices.graphicsFamily = i;
			indices.presentFamily = i;
			break;
		}
	}
	if (!indices.graphicsFamily.has_value()) {
		for (uint32_t i = 0; i < count; ++i) {
			if (hasFlag(i, VK_QUEUE_GRAPHICS_BIT)) {
				indices.graphicsFamily = i;
				break;
			}
		}
		for (uint32_t i = 0; i < count; ++i) {
			if (canPresent[i]) {
				indices.presentFamily = i;
				break;
			}
		}
	}

	// Compute
	for (uint32_t i = 0; i < count; ++i) {
		if (hasFlag(i, VK_QUEUE_COMPUTE_BIT) && !hasFlag(i, VK_QUEUE_GRAPHICS_BIT)) {
			indices.computeFamily = i;
			break;
		}
	}
	if (!indices.computeFamily) {
		for (uint32_t i = 0; i < count; ++i) {
			if (hasFlag(i, VK_QUEUE_COMPUTE_BIT) && i != indices.graphicsFamily) {
				indices.computeFamily = i;
				break;
			}
		}
	}
	if (!indices.computeFamily.has_value()) {
		for (uint32_t i = 0; i < count; ++i) {
			if (hasFlag(i, VK_QUEUE_COMPUTE_BIT)) {
				indices.computeFamily = i;
				break;
			}
		}
	}

	return indices;
}

} // namespace lm
