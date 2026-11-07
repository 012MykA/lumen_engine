#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

namespace lm {

class VulkanSampler final {
public:
	VulkanSampler(VkDevice device, float maxAnisotropy);
	~VulkanSampler();

	VulkanSampler(const VulkanSampler &) = delete;
	VulkanSampler &operator=(const VulkanSampler &) = delete;

	VkSampler GetVkSampler() const { return m_Sampler; }

private:
	VkDevice m_Device{ VK_NULL_HANDLE };
	VkSampler m_Sampler{ VK_NULL_HANDLE };
};

} // namespace lm
