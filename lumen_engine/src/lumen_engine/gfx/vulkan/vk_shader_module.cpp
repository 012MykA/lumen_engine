#include "vk_shader_module.hpp"

namespace lm {

VulkanShaderModule::VulkanShaderModule(VkDevice device, std::span<const uint32_t> code) :
		m_Device{ device } {
	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size() * sizeof(uint32_t),
		.pCode = code.data(),
	};
	CHECK_VK(vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_ShaderModule));
}

VulkanShaderModule::~VulkanShaderModule() {
	vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);
}

} // namespace lm
