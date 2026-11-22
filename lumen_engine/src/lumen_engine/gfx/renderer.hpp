#pragma once

#include "lumen_engine/core/window.hpp"
#include "lumen_engine/gfx/mesh.hpp"
#include "lumen_engine/gfx/primitives.hpp"
#include "lumen_engine/gfx/vulkan/vk_buffer.hpp"
#include "lumen_engine/gfx/vulkan/vk_context.hpp"
#include "lumen_engine/gfx/vulkan/vk_descriptors.hpp"
#include "lumen_engine/gfx/vulkan/vk_frame_data.hpp"
#include "lumen_engine/gfx/vulkan/vk_global_ubo.hpp"
#include "lumen_engine/gfx/vulkan/vk_image.hpp"
#include "lumen_engine/gfx/vulkan/vk_pipeline.hpp"
#include "lumen_engine/gfx/vulkan/vk_pipeline_cache.hpp"
#include "lumen_engine/gfx/vulkan/vk_push_constants.hpp"
#include "lumen_engine/gfx/vulkan/vk_sampler.hpp"
#include "lumen_engine/gfx/vulkan/vk_swapchain.hpp"
#include "lumen_engine/gfx/vulkan/vk_uploader.hpp"
#include "lumen_engine/scene/camera.hpp"

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <optional>

namespace lm {

struct DrawItem {
	const Mesh *mesh;
	glm::mat4 model;
};

class Renderer {
	static constexpr uint32_t k_FramesInFlight{ 3 };
	static constexpr VkFormat k_DepthFormat{ VK_FORMAT_D32_SFLOAT };

	struct FrameContext {
		VkCommandBuffer cmd{ VK_NULL_HANDLE };
		VkImage image{ VK_NULL_HANDLE };
		VkImageView imageView{ VK_NULL_HANDLE };
		VkExtent2D extent{ 0, 0 };
		uint32_t imageIndex{ 0 };
		uint32_t frameIndex{ 0 };
	};

public:
	Renderer(const Window &window);
	~Renderer();

	Mesh CreateMesh(const MeshData &data);

	void BeginScene(const Camera &camera, float time, float dt);
	void Submit(const Mesh &mesh, const glm::mat4 &model);

	bool BeginFrame();
	void RenderScene();
	void EndFrame();

	void OnResize();
	void WaitIdle() const;

private:
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

	std::optional<DescriptorAllocator> m_DescriptorAllocator;

	VkDescriptorSetLayout m_GlobalSetLayout{ VK_NULL_HANDLE }; // set 0
	VkDescriptorSetLayout m_MaterialSetLayout{ VK_NULL_HANDLE }; // set 1

	std::array<VulkanBuffer, k_FramesInFlight> m_GlobalUBOs;
	std::array<VkDescriptorSet, k_FramesInFlight> m_GlobalSets{};
	GlobalUBO m_SceneData{};

	VkDescriptorSet m_TextureSet{ VK_NULL_HANDLE };
	VulkanImage m_Texture;
	std::optional<VulkanSampler> m_Sampler;

	std::array<FrameData, k_FramesInFlight> m_Frames{};
	uint32_t m_FrameIndex{ 0 };

	FrameContext m_FrameCtx{};
	bool m_FrameActive{ false };
	std::vector<DrawItem> m_DrawList;

	bool m_NeedsResize{ false };
};

} // namespace lm
