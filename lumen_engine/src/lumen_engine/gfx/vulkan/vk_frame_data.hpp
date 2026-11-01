#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

namespace lm {

struct FrameData {
	VkCommandPool pool{ VK_NULL_HANDLE };
	VkCommandBuffer cmd{ VK_NULL_HANDLE };
	VkFence inFlight{ VK_NULL_HANDLE };
	VkSemaphore imageAvailable{ VK_NULL_HANDLE };
};

} // namespace lm
