#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"
#include "lumen_engine/rhi/vulkan_backend/vulkan_validation.hpp"

#include <stdexcept>

namespace lm::rhi {

ShaderHandle VulkanRenderingDevice::CreateShader(size_t codeSize, const void *code) {
	if (codeSize % 4 != 0) {
		throw std::runtime_error("CreateShader(): wrong SPIR-V code size. It must be alligned to 4 bytes");
	}

	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = codeSize,
		.pCode = reinterpret_cast<const uint32_t *>(code),
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
