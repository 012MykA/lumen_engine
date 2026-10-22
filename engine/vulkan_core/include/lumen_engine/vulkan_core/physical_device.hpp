#pragma once

#include <volk.h>

#include <functional>
#include <optional>
#include <vector>

namespace lmvk {

// --- Queue Family Indices

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> transferFamily;

	bool IsComplete(bool requirePresent = true) const {
		return graphicsFamily && computeFamily && (!requirePresent || presentFamily);
	}
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

// --- Device Features ---

struct FeatureChain {
	VkPhysicalDeviceVulkan11Features f11{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
	VkPhysicalDeviceVulkan12Features f12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	VkPhysicalDeviceVulkan13Features f13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	VkPhysicalDeviceFeatures2 f2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };

	FeatureChain() {
		f2.pNext = &f13;
		f13.pNext = &f12;
		f12.pNext = &f11;
	}

	FeatureChain(const FeatureChain &) = delete;
	FeatureChain &operator=(const FeatureChain &) = delete;
};

void EnableRequiredFeatures(FeatureChain &chain);

bool HasRequiredFeatures(const FeatureChain &queried);

// --- Physical Device ---

using PresentSupportFn = std::function<bool(VkPhysicalDevice, uint32_t /*queueFamily*/)>;

struct PhysicalDeviceDesc {
	std::vector<const char *> requiredExtensions;
	PresentSupportFn presentSupport; // optional
};

class PhysicalDevice final {
public:
	PhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const PhysicalDeviceDesc &desc);

	VkPhysicalDevice Handle() const { return m_Device; }
	const QueueFamilyIndices &GetQueueFamilies() const { return m_QueueFamilies; }
	const VkPhysicalDeviceProperties &GetProperties() const { return m_Properties; }
	const std::vector<const char *> &GetEnabledExtensions() const { return m_EnabledExtensions; }

private:
	VkPhysicalDevice m_Device = VK_NULL_HANDLE;
	QueueFamilyIndices m_QueueFamilies;
	VkPhysicalDeviceProperties m_Properties{};
	std::vector<const char *> m_EnabledExtensions;
};

} // namespace lmvk
