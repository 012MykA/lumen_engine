#include "lumen_engine/vulkan_core/physical_device.hpp"

#include "lumen_engine/vulkan_core/check_vk.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace lmvk {

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

	for (uint32_t i = 0; i < count; ++i) {
		const auto &family = families[i];
		bool hasGraphics = (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
		bool hasCompute = (family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
		bool hasTransferOnly = (family.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0 && !hasGraphics && !hasCompute;

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (hasGraphics && presentSupport && !indices.graphicsFamily) {
			indices.graphicsFamily = i;
			indices.presentFamily = i;
		}
		if (hasCompute && (!indices.computeFamily || !hasGraphics)) {
			indices.computeFamily = i;
		}
		if (hasTransferOnly) {
			indices.transferFamily = i;
		}
	}

	if (!indices.graphicsFamily) {
		for (uint32_t i = 0; i < count; ++i) {
			if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
				break;
			}
		}
	}
	if (!indices.presentFamily) {
		for (uint32_t i = 0; i < count; ++i) {
			VkBool32 support = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &support);
			if (support) {
				indices.presentFamily = i;
				break;
			}
		}
	}

	return indices;
}

namespace {

bool SupportsExtensions(VkPhysicalDevice device, const std::vector<const char *> &required) {
	uint32_t count = 0;
	CHECK_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr));
	std::vector<VkExtensionProperties> available(count);
	CHECK_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data()));

	return std::ranges::all_of(required, [&](const char *name) {
		return std::ranges::any_of(available, [&](const auto &p) { return std::strcmp(p.extensionName, name) == 0; });
	});
}

} // namespace

void EnableRequiredFeatures(FeatureChain &chain) {
	chain.f2.features.samplerAnisotropy = VK_TRUE;
	chain.f13.dynamicRendering = VK_TRUE;
}

bool HasRequiredFeatures(const FeatureChain &q) {
	return q.f2.features.samplerAnisotropy &&
			q.f13.dynamicRendering;
}

PhysicalDevice::PhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const PhysicalDeviceDesc &desc) {
	uint32_t count = 0;
	CHECK_VK(vkEnumeratePhysicalDevices(instance, &count, nullptr));
	if (count == 0) {
		throw std::runtime_error("No Vulkan-capable GPUs found");
	}
	std::vector<VkPhysicalDevice> devices(count);
	CHECK_VK(vkEnumeratePhysicalDevices(instance, &count, devices.data()));

	auto it = std::ranges::find_if(devices, [&](VkPhysicalDevice device) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);

		FeatureChain features;
		vkGetPhysicalDeviceFeatures2(device, &features.f2);

		return props.apiVersion >= VK_API_VERSION_1_3 &&
				SupportsExtensions(device, desc.requiredExtensions) &&
				HasRequiredFeatures(features) &&
				FindQueueFamilies(device, surface).IsComplete();
	});
	if (it == devices.end()) {
		throw std::runtime_error("No suitable GPU found");
	}

	m_Device = *it;
	m_QueueFamilies = FindQueueFamilies(m_Device, surface);
	m_EnabledExtensions = desc.requiredExtensions;
	vkGetPhysicalDeviceProperties(m_Device, &m_Properties);
}

} // namespace lmvk
