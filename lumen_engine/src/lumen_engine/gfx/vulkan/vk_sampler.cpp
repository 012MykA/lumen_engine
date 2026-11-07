#include "lumen_engine/gfx/vulkan/vk_sampler.hpp"

namespace lm {

VulkanSampler::VulkanSampler(VkDevice device, float maxAnisotropy) : m_Device{ device } {
	VkSamplerCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = maxAnisotropy,
		.minLod = 0.0f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	CHECK_VK(vkCreateSampler(m_Device, &info, nullptr, &m_Sampler));
}

VulkanSampler::~VulkanSampler() {
	vkDestroySampler(m_Device, m_Sampler, nullptr);
}

} // namespace lm
