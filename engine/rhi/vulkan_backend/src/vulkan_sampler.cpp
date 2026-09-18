#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"
#include "vulkan_translate.hpp"

namespace lm::rhi {

SamplerHandle lm::rhi::VulkanRenderingDevice::CreateSampler(const SamplerDesc &desc) {
	VulkanSampler sampler;

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

	sampler.sampler = CreateVkSampler(props.limits.maxSamplerAnisotropy);

	return m_SamplerPool.Insert(sampler);
}

void VulkanRenderingDevice::DestroySampler(SamplerHandle handle) {
	if (VulkanSampler *sampler = m_SamplerPool.Get(handle)) {
		sampler->Destroy(m_Device);
		m_SamplerPool.Remove(handle);
	}
}

void *VulkanRenderingDevice::GetVkSampler(SamplerHandle handle) const {
	const VulkanSampler *sampler = m_SamplerPool.Get(handle);
	return sampler ? sampler->sampler : nullptr;
}

VkSampler VulkanRenderingDevice::CreateVkSampler(float maxAnisotropy) {
	VkSamplerCreateInfo samplerInfo{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = maxAnisotropy,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
	};
	VkSampler sampler;
	CHECK_VK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &sampler));
	return sampler;
}

} // namespace lm::rhi
