#pragma once

#include "lumen_engine/rhi/types.hpp"

#include <vulkan/vulkan.h>

namespace lm::rhi {

VkMemoryPropertyFlags TranslateMemoryUsage(MemoryUsage usage);

VkBufferUsageFlags TranslateBufferUsage(BufferUsage usage);

} // namespace lm::rhi
