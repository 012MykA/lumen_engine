#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <cstdint>
#include <span>

namespace lm {

class VulkanShaderModule final {
public:
	VulkanShaderModule(VkDevice device, std::span<const uint32_t> code);
	~VulkanShaderModule();

	VkShaderModule GetVkShaderModule() const { return m_ShaderModule; }

private:
	VkDevice m_Device{ VK_NULL_HANDLE };
	VkShaderModule m_ShaderModule{ VK_NULL_HANDLE };
};

} // namespace lm
