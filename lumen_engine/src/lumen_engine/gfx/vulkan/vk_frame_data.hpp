#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

namespace lm {

struct FrameData {
	VkCommandBuffer cmd;
	VkFence inFlight;
	VkSemaphore imageAvailable;
};

} // namespace lm
