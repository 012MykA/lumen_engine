#pragma once

#include "lumen_engine/core/window.hpp"
#include "lumen_engine/gfx/vulkan/vk_context.hpp"
#include "lumen_engine/gfx/vulkan/vk_frame_data.hpp"
#include "lumen_engine/gfx/vulkan/vk_swapchain.hpp"

#include <array>
#include <cstdint>

namespace lm {

class Renderer {
	static constexpr uint32_t k_FramesInFlight{ 3 };

public:
	Renderer(const Window &window);
	~Renderer();

	void OnRender();

	void OnResize(WindowExtent extent);

private:
	void RecordCommands(VkCommandBuffer cmd, uint32_t imageIndex);

	void CreateFrames();
	void DestroyFrames();

private:
	const Window &m_Window;
	VulkanContext m_Context;
	VulkanSwapchain m_Swapchain;
	std::array<FrameData, k_FramesInFlight> m_Frames{};
	uint32_t m_FrameIndex{ 0 };
	bool m_NeedsResize{ false };
};

} // namespace lm
