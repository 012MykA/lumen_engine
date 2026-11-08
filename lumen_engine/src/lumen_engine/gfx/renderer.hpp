#pragma once

#include "lumen_engine/core/window.hpp"
#include "lumen_engine/gfx/vulkan/vk_buffer.hpp"
#include "lumen_engine/gfx/vulkan/vk_context.hpp"
#include "lumen_engine/gfx/vulkan/vk_frame_data.hpp"
#include "lumen_engine/gfx/vulkan/vk_image.hpp"
#include "lumen_engine/gfx/vulkan/vk_pipeline.hpp"
#include "lumen_engine/gfx/vulkan/vk_pipeline_cache.hpp"
#include "lumen_engine/gfx/vulkan/vk_sampler.hpp"
#include "lumen_engine/gfx/vulkan/vk_swapchain.hpp"
#include "lumen_engine/gfx/vulkan/vk_uploader.hpp"

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <optional>

namespace lm {

class Renderer {
	static constexpr uint32_t k_FramesInFlight{ 3 };
	static constexpr VkFormat k_DepthFormat{ VK_FORMAT_D32_SFLOAT };

public:
	Renderer(const Window &window);
	~Renderer();

	void SetViewProjection(const glm::mat4 &vp) { m_ViewProj = vp; }

	void OnRender();
	void OnResize();

private:
	void RecordCommands(VkCommandBuffer cmd, uint32_t imageIndex);

	void CreateFrames();
	void CreateDescriptors();
	void CreatePipelines();
	void CreateDepthImage();
	
	void CreateTexture();

	void DestroyFrames();

private:
	const Window &m_Window;
	VulkanContext m_Context;
	VulkanSwapchain m_Swapchain;
	VulkanPipelineCache m_PipelineCache;
	std::optional<VulkanPipeline> m_Pipeline;
	VulkanUploader m_Uploader;
	VulkanImage m_DepthImage;

	std::array<FrameData, k_FramesInFlight> m_Frames{};
	uint32_t m_FrameIndex{ 0 };
	bool m_NeedsResize{ false };

	glm::mat4 m_ViewProj{ 1.0f };
	uint32_t m_IndexCount{ 0 };

	VulkanBuffer m_VertexBuffer, m_IndexBuffer;
	VulkanImage m_Texture;
	std::optional<VulkanSampler> m_Sampler;
	VkDescriptorSetLayout m_SetLayout{ VK_NULL_HANDLE };
	VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };
	VkDescriptorSet m_TextureSet{ VK_NULL_HANDLE };
};

} // namespace lm
