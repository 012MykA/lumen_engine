#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"
#include "lumen_engine/rhi/vulkan_backend/vulkan_validation.hpp"

#include <stdexcept>

namespace lm::rhi {

ShaderHandle VulkanRenderingDevice::CreateShader(std::span<const uint32_t> spirvCode) {
	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = spirvCode.size_bytes(),
		.pCode = spirvCode.data(),
	};
	VkShaderModule shaderModule;
	CHECK_VK(vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule));

	return m_ShaderPool.Insert(shaderModule);
}

void VulkanRenderingDevice::DestroyShader(ShaderHandle handle) {
	if (VkShaderModule *shader = m_ShaderPool.Get(handle)) {
		vkDestroyShaderModule(m_Device, *shader, nullptr);
		m_ShaderPool.Remove(handle);
	}
}

void *VulkanRenderingDevice::GetVkShaderModule(ShaderHandle handle) const {
	const VkShaderModule *shader = m_ShaderPool.Get(handle);
	return shader ? *shader : nullptr;
}

} // namespace lm::rhi
