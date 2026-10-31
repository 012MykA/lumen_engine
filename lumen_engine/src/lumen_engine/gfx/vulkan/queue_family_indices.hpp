#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <cstdint>
#include <optional>

namespace lm {

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool IsComplete() const {
		return graphicsFamily.has_value() &&
				presentFamily.has_value() &&
				computeFamily.has_value();
	}
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

} // namespace lm
