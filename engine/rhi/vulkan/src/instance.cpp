#include "lumen_engine/rhi/vulkan/vulkan_device.hpp"
#include "vulkan_validation.hpp"

namespace lm::rhi {

void VulkanDevice::CreateInstance() {
	constexpr VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "No name",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Lumen Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = k_VulkanApiVersion,
	};

	const std::vector<const char *> enabledLayers = FilterSupportedInstanceLayers(k_InstanceLayers, /*required=*/true);
	const std::vector<const char *> enabledExtensions = FilterSupportedInstanceExtensions(k_InstanceExtensions, /*required=*/true);

	const VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
		.ppEnabledLayerNames = enabledLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
	};

	CHECK_VK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
}

void VulkanDevice::DestroyInstance() {
	vkDestroyInstance(m_Instance, nullptr);
}

} // namespace lm::rhi
