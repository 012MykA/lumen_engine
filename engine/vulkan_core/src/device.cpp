#include "lumen_engine/vulkan_core/device.hpp"

#include "lumen_engine/vulkan_core/check_vk.hpp"

#include <set>
#include <utility>

namespace lm::vk {

Device::Device(const PhysicalDevice &physicalDevice) {
	const QueueFamilyIndices &indices = physicalDevice.GetQueueFamilies();

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value(),
		indices.computeFamily.value(),
	};

	float priority = 1.0f;
	for (uint32_t family : uniqueFamilies) {
		queueCreateInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = family,
				.queueCount = 1,
				.pQueuePriorities = &priority,
		});
	}

	VkPhysicalDeviceVulkan11Features f11{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
	f11.shaderDrawParameters = VK_TRUE;

	VkPhysicalDeviceVulkan12Features f12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &f11 };
	f12.bufferDeviceAddress = VK_TRUE;
	f12.descriptorIndexing = VK_TRUE;
	f12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	f12.runtimeDescriptorArray = VK_TRUE;
	f12.descriptorBindingPartiallyBound = VK_TRUE;
	f12.descriptorBindingVariableDescriptorCount = VK_TRUE;
	f12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	f12.timelineSemaphore = VK_TRUE;

	VkPhysicalDeviceVulkan13Features f13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &f12 };
	f13.dynamicRendering = VK_TRUE;
	f13.synchronization2 = VK_TRUE;

	VkPhysicalDeviceFeatures2 features2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &f13 };
	features2.features.samplerAnisotropy = VK_TRUE;

	const auto &extensions = physicalDevice.GetEnabledExtensions();
	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features2,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
	};
	CHECK_VK(vkCreateDevice(physicalDevice.Handle(), &createInfo, nullptr, &m_Device));
	volkLoadDevice(m_Device);

	vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
	vkGetDeviceQueue(m_Device, indices.computeFamily.value(), 0, &m_ComputeQueue);
}

Device::~Device() {
	if (m_Device != VK_NULL_HANDLE) {
		vkDestroyDevice(m_Device, nullptr);
	}
}

Device::Device(Device &&other) noexcept
		: m_Device(std::exchange(other.m_Device, VK_NULL_HANDLE)),
		  m_GraphicsQueue(other.m_GraphicsQueue),
		  m_PresentQueue(other.m_PresentQueue),
		  m_ComputeQueue(other.m_ComputeQueue) {}

Device &Device::operator=(Device &&other) noexcept {
	if (this != &other) {
		if (m_Device != VK_NULL_HANDLE) {
			vkDestroyDevice(m_Device, nullptr);
		}
		m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
		m_GraphicsQueue = other.m_GraphicsQueue;
		m_PresentQueue = other.m_PresentQueue;
		m_ComputeQueue = other.m_ComputeQueue;
	}
	return *this;
}

void Device::WaitIdle() const {
	CHECK_VK(vkDeviceWaitIdle(m_Device));
}

} // namespace lm::vk
