#pragma once

#include <volk.h>

#include <optional>
#include <vector>

namespace lm::vk {

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> transferFamily;

	bool IsComplete() const {
		return graphicsFamily && presentFamily && computeFamily;
	}
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

struct PhysicalDeviceDesc {
	std::vector<const char *> requiredExtensions;
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

} // namespace lm::vk
