#include "lumen_engine/rhi/vulkan/vulkan_device.hpp"
#include "vulkan_validation.hpp"

#include <cstdint>
#include <set>

namespace lm::rhi {

void VulkanDevice::CreateLogicalDevice() {
	// Queues
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		m_QueueFamilyIndices.graphicsFamily.value(),
		m_QueueFamilyIndices.presentFamily.value(),
		m_QueueFamilyIndices.computeFamily.value(),
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
		});
	}

	// Extensions
	auto enabledExtensions = FilterSupportedDeviceExtensions(m_PhysicalDevice, k_DeviceExtensions, /*required=*/true);

	// Features
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
	vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);
	features2.features.samplerAnisotropy = VK_TRUE;
	vulkan11Features.shaderDrawParameters = VK_TRUE;
	vulkan13Features.dynamicRendering = VK_TRUE;
	extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features2,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
		.pEnabledFeatures = nullptr,
	};
	CHECK_VK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

	vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.presentFamily.value(), 0, &m_PresentQueue);
	vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.computeFamily.value(), 0, &m_ComputeQueue);
}

void VulkanDevice::DestroyLogicalDevice() {
	vkDestroyDevice(m_Device, nullptr);
}

} // namespace lm::rhi
