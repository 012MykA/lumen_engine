#include "lumen_engine/rhi/vulkan_backend/vulkan_device.hpp"
#include "vulkan_validation.hpp"

#include <cstdint>

namespace lm::rhi {

std::vector<VkLayerProperties> VulkanDevice::EnumerateInstanceLayers() const {
	uint32_t count = 0;
	CHECK_VK(vkEnumerateInstanceLayerProperties(&count, nullptr));
	std::vector<VkLayerProperties> available(count);
	CHECK_VK(vkEnumerateInstanceLayerProperties(&count, available.data()));
	return available;
}

std::vector<VkExtensionProperties> VulkanDevice::EnumerateInstanceExtensions() const {
	uint32_t count = 0;
	CHECK_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
	std::vector<VkExtensionProperties> available(count);
	CHECK_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data()));
	return available;
}

std::vector<VkExtensionProperties> VulkanDevice::EnumerateDeviceExtensions(VkPhysicalDevice device) const {
	uint32_t count = 0;
	CHECK_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr));
	std::vector<VkExtensionProperties> available(count);
	CHECK_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data()));
	return available;
}

std::vector<VkPhysicalDevice> VulkanDevice::EnumeratePhysicalDevices() const {
	uint32_t count = 0;
	CHECK_VK(vkEnumeratePhysicalDevices(m_Instance, &count, nullptr));
	std::vector<VkPhysicalDevice> devices(count);
	CHECK_VK(vkEnumeratePhysicalDevices(m_Instance, &count, devices.data()));
	return devices;
}

} // namespace lm::rhi
