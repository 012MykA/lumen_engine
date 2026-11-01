#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

namespace lm {

struct DeviceFeatureChain {
	DeviceFeatureChain() = default;
	DeviceFeatureChain(const DeviceFeatureChain &) = delete;
	DeviceFeatureChain &operator=(const DeviceFeatureChain &) = delete;

	VkPhysicalDeviceFeatures2 features2{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &extendedDynamicState,
	};

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicState{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
		.pNext = &vk13
	};

	VkPhysicalDeviceVulkan13Features vk13{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &vk11
	};

	VkPhysicalDeviceVulkan11Features vk11{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
		.pNext = nullptr
	};

	void QueryFor(VkPhysicalDevice device) {
		vkGetPhysicalDeviceFeatures2(device, &features2);
	}
};

} // namespace lm
