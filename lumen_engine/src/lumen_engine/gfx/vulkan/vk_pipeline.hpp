#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <span>
#include <vector>

namespace lm {

class VulkanPipeline final {
public:
	VulkanPipeline(VkDevice device, VkPipelineLayout layout, VkPipeline pipeline);
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline &) = delete;
	VulkanPipeline &operator=(const VulkanPipeline &) = delete;
	VulkanPipeline(VulkanPipeline &&other) noexcept;
	VulkanPipeline &operator=(VulkanPipeline &&other) noexcept;

	VkPipelineLayout GetVkPipelineLayout() const { return m_Layout; }
	VkPipeline GetVkPipeline() const { return m_Pipeline; }

private:
	void Destroy();

private:
	VkDevice m_Device{ VK_NULL_HANDLE };
	VkPipelineLayout m_Layout{ VK_NULL_HANDLE };
	VkPipeline m_Pipeline{ VK_NULL_HANDLE };
};

class GraphicsPipelineBuilder {
public:
	GraphicsPipelineBuilder &SetShader(VkShaderModule shader);

	GraphicsPipelineBuilder &SetVertexInput(
			std::span<const VkVertexInputBindingDescription> bindingDesc,
			std::span<const VkVertexInputAttributeDescription> attribDesc);

	GraphicsPipelineBuilder &SetCullMode(VkCullModeFlags mode, VkFrontFace front);
	GraphicsPipelineBuilder &SetColorFormat(VkFormat format);
	GraphicsPipelineBuilder &SetDepthFormat(VkFormat format);

	template <typename T>
	GraphicsPipelineBuilder &SetPushConstants(VkShaderStageFlags stage, uint32_t offset = 0) {
		static_assert(sizeof(T) <= 128, "maxPushConstantSize = 128");
		static_assert(sizeof(T) % 4 == 0, "Alligment");
		m_PushConstant = VkPushConstantRange{
			.stageFlags = stage,
			.offset = offset,
			.size = sizeof(T),
		};
		return *this;
	}

	VulkanPipeline Build(VkDevice device, VkPipelineCache cache) const;

private:
	VkShaderModule m_Shader{ VK_NULL_HANDLE };
	VkFormat m_ColorFormat{ VK_FORMAT_UNDEFINED };
	VkCullModeFlags m_CullMode{ VK_CULL_MODE_NONE };
	VkFrontFace m_FrontFace{ VK_FRONT_FACE_COUNTER_CLOCKWISE };
	std::vector<VkVertexInputBindingDescription> m_Bindings;
	std::vector<VkVertexInputAttributeDescription> m_Attributes;
	VkPushConstantRange m_PushConstant{};
	VkFormat m_DepthFormat{ VK_FORMAT_UNDEFINED };
};

} // namespace lm
