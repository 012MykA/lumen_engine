#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"
#include "lumen_engine/rhi/vulkan_backend/vulkan_validation.hpp"
#include "vulkan_translate.hpp"

#include <array>
#include <stdexcept>

namespace lm::rhi {

PipelineHandle VulkanRenderingDevice::CreatePipeline(const GraphicsPipelineDesc &desc) {
	if (!desc.pipelineLayout) {
		throw std::runtime_error("CreatePipeline(): pipelineLayout must be set");
	}

	// Shader stages
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.reserve(desc.shaderStages.size());

	for (const auto &s : desc.shaderStages) {
		VkShaderModule *module = m_ShaderPool.Get(s.shaderModule);
		VkPipelineShaderStageCreateInfo stage{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = TranslateShaderStageBits(s.stage),
			.module = module ? *module : nullptr,
			.pName = s.entryPoint.c_str(),
		};
		shaderStages.push_back(stage);
	}

	// Vertex input
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	bindingDescriptions.reserve(desc.vertexBindings.size());

	for (const auto &b : desc.vertexBindings) {
		bindingDescriptions.push_back(VkVertexInputBindingDescription{
				.binding = b.binding,
				.stride = b.stride,
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		});

		for (const auto &attr : b.attributtes) {
			attributeDescriptions.push_back(
					VkVertexInputAttributeDescription{
							.location = attr.location,
							.binding = b.binding,
							.format = TranslateVertexAttribFormat(attr.format),
							.offset = attr.offset,
					});
		}
	}

	VkPipelineVertexInputStateCreateInfo vertexInputState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
		.pVertexBindingDescriptions = bindingDescriptions.data(),
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data(),
	};

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = TranslatePrimitiveTopology(desc.topology),
	};

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = TranslateCullMode(desc.cullMode),
		.frontFace = TranslateFrontFace(desc.frontFace),
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f,
	};

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
	};

	// Depth Stencil
	VkPipelineDepthStencilStateCreateInfo depthState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = desc.depthStencil.depthTestEnable ? VK_TRUE : VK_FALSE,
		.depthWriteEnable = desc.depthStencil.depthWriteEnable ? VK_TRUE : VK_FALSE,
		.depthCompareOp = TranslateCompareOp(desc.depthStencil.depthCompareOp),
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
	};

	// Color blend attachemnts
	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = desc.colorBlend.blendEnable ? VK_TRUE : VK_FALSE,
		.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};

	// Dynamic state
	std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};

	// Dynamic rendering
	std::vector<VkFormat> colorAttachmentFormats;
	colorAttachmentFormats.reserve(desc.colorAttachmentFormats.size());
	for (const auto &format : desc.colorAttachmentFormats) {
		colorAttachmentFormats.push_back(TranslateFormat(format));
	}

	VkPipelineRenderingCreateInfo pipelineRenderingInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentFormats.size()),
		.pColorAttachmentFormats = colorAttachmentFormats.data(),
		.depthAttachmentFormat = TranslateFormat(desc.depthAttachmentFormat),
	};

	VkGraphicsPipelineCreateInfo graphicsPipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipelineRenderingInfo,
		.stageCount = static_cast<uint32_t>(shaderStages.size()),
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInputState,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depthState,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = reinterpret_cast<VkPipelineLayout>(desc.pipelineLayout),
		.renderPass = nullptr,
	};

	VkPipeline pipeline;
	CHECK_VK(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &pipeline));

	return m_PipelinePool.Insert(pipeline);
}

void VulkanRenderingDevice::DestroyPipeline(PipelineHandle handle) {
	if (VkPipeline *pipeline = m_PipelinePool.Get(handle)) {
		vkDestroyPipeline(m_Device, *pipeline, nullptr);
		m_PipelinePool.Remove(handle);
	}
}

void *VulkanRenderingDevice::GetVkPipeline(PipelineHandle handle) const {
	const VkPipeline *pipeline = m_PipelinePool.Get(handle);
	return pipeline ? *pipeline : nullptr;
}

} // namespace lm::rhi
