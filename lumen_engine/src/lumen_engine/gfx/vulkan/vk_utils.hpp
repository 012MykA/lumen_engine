#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace lm {

void TransitionImage(VkCommandBuffer cmd, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
		VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess,
		VkImageAspectFlags aspect);

std::vector<uint32_t> ReadSpirV(const std::filesystem::path &path);

} // namespace lm
