#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"
#include "vulkan_translate.hpp"

namespace lm::rhi {

SamplerHandle VulkanRenderingDevice::CreateSampler(const SamplerDesc &desc) {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

	const float maxAnisotropy = (desc.maxAnisotropy == SamplerDesc::k_AutoAnisotropy)
			? props.limits.maxSamplerAnisotropy
			: desc.maxAnisotropy;

	const VkSamplerCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = TranslateFilter(desc.magFilter),
		.minFilter = TranslateFilter(desc.minFilter),
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = TranslateAddressMode(desc.addressModeU),
		.addressModeV = TranslateAddressMode(desc.addressModeV),
		.addressModeW = TranslateAddressMode(desc.addressModeW),
		.mipLodBias = 0.0f,
		.anisotropyEnable = desc.anisotropyEnabled ? VK_TRUE : VK_FALSE,
		.maxAnisotropy = maxAnisotropy,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0.0f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	VkSampler sampler;
	CHECK_VK(vkCreateSampler(m_Device, &createInfo, nullptr, &sampler));

	return m_SamplerPool.Insert(sampler);
}

void VulkanRenderingDevice::DestroySampler(SamplerHandle handle) {
	if (VkSampler *sampler = m_SamplerPool.Get(handle)) {
		vkDestroySampler(m_Device, *sampler, nullptr);
		m_SamplerPool.Remove(handle);
	}
}

void *VulkanRenderingDevice::GetVkSampler(SamplerHandle handle) const {
	const VkSampler *sampler = m_SamplerPool.Get(handle);
	return sampler ? *sampler : nullptr;
}

} // namespace lm::rhi
