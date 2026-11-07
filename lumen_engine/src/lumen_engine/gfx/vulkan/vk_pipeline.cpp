#include "lumen_engine/gfx/vulkan/vk_pipeline.hpp"

#include <array>
#include <utility> // for std::exchange

namespace lm {

VulkanPipeline::VulkanPipeline(VkDevice device, VkPipelineLayout layout, VkPipeline pipeline) :
		m_Device{ device },
		m_Layout{ layout },
		m_Pipeline{ pipeline } {}

VulkanPipeline::~VulkanPipeline() {
	Destroy();
}

VulkanPipeline::VulkanPipeline(VulkanPipeline &&other) noexcept :
		m_Device{ std::exchange(other.m_Device, VK_NULL_HANDLE) },
		m_Layout{ std::exchange(other.m_Layout, VK_NULL_HANDLE) },
		m_Pipeline{ std::exchange(other.m_Pipeline, VK_NULL_HANDLE) } {}

VulkanPipeline &VulkanPipeline::operator=(VulkanPipeline &&other) noexcept {
	if (this != &other) {
		Destroy();
		m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
		m_Layout = std::exchange(other.m_Layout, VK_NULL_HANDLE);
		m_Pipeline = std::exchange(other.m_Pipeline, VK_NULL_HANDLE);
	}
	return *this;
}

void VulkanPipeline::Destroy() {
	if (m_Device == VK_NULL_HANDLE) {
		return;
	}
	vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
	m_Device = VK_NULL_HANDLE;
	m_Pipeline = VK_NULL_HANDLE;
	m_Layout = VK_NULL_HANDLE;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetShader(VkShaderModule shader) {
	m_Shader = shader;
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetColorFormat(VkFormat format) {
	m_ColorFormat = format;
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetCullMode(VkCullModeFlags mode, VkFrontFace front) {
	m_CullMode = mode;
	m_FrontFace = front;
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetDepthFormat(VkFormat format) {
	m_DepthFormat = format;
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetVertexInput(
		std::span<const VkVertexInputBindingDescription> bindingDesc,
		std::span<const VkVertexInputAttributeDescription> attribDesc) {
	m_Bindings.assign(bindingDesc.begin(), bindingDesc.end());
	m_Attributes.assign(attribDesc.begin(), attribDesc.end());
	return *this;
}

VulkanPipeline GraphicsPipelineBuilder::Build(VkDevice device, VkPipelineCache cache) const {
	VkPipelineLayoutCreateInfo layoutInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pushConstantRangeCount = m_PushConstant.size > 0 ? 1u : 0u,
		.pPushConstantRanges = m_PushConstant.size > 0 ? &m_PushConstant : nullptr,
	};
	VkPipelineLayout layout{ VK_NULL_HANDLE };
	CHECK_VK(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout));

	std::array stages = {
		VkPipelineShaderStageCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = m_Shader,
				.pName = "vertMain",
		},
		VkPipelineShaderStageCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = m_Shader,
				.pName = "fragMain",
		},
	};

	VkPipelineVertexInputStateCreateInfo vertexInput{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = static_cast<uint32_t>(m_Bindings.size()),
		.pVertexBindingDescriptions = m_Bindings.data(),
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_Attributes.size()),
		.pVertexAttributeDescriptions = m_Attributes.data(),
	};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};
	VkPipelineViewportStateCreateInfo viewport{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};
	VkPipelineRasterizationStateCreateInfo raster{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = m_CullMode,
		.frontFace = m_FrontFace,
		.lineWidth = 1.0f,
	};
	VkPipelineMultisampleStateCreateInfo msaa{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	};
	VkPipelineDepthStencilStateCreateInfo depthStencil{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = (m_DepthFormat != VK_FORMAT_UNDEFINED) ? VK_TRUE : VK_FALSE,
		.depthWriteEnable = (m_DepthFormat != VK_FORMAT_UNDEFINED) ? VK_TRUE : VK_FALSE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
	};
	VkPipelineColorBlendAttachmentState blendAttachment{
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	VkPipelineColorBlendStateCreateInfo blend{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &blendAttachment,
	};
	constexpr std::array dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};
	VkPipelineRenderingCreateInfo rendering{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &m_ColorFormat,
		.depthAttachmentFormat = m_DepthFormat,
	};

	VkGraphicsPipelineCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &rendering,
		.stageCount = static_cast<uint32_t>(stages.size()),
		.pStages = stages.data(),
		.pVertexInputState = &vertexInput,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewport,
		.pRasterizationState = &raster,
		.pMultisampleState = &msaa,
		.pDepthStencilState = &depthStencil,
		.pColorBlendState = &blend,
		.pDynamicState = &dynamic,
		.layout = layout,
	};

	VkPipeline pipeline{ VK_NULL_HANDLE };
	const VkResult res = vkCreateGraphicsPipelines(device, cache, 1, &createInfo, nullptr, &pipeline);
	if (res != VK_SUCCESS) {
		vkDestroyPipelineLayout(device, layout, nullptr);
		CHECK_VK(res);
	}

	return VulkanPipeline{ device, layout, pipeline };
}

} // namespace lm
