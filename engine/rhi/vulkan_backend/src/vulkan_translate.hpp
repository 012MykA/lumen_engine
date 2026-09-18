#pragma once

#include "lumen_engine/rhi/types.hpp"

#include <vulkan/vulkan.h>

namespace lm::rhi {

VkMemoryPropertyFlags TranslateMemoryUsage(MemoryUsage usage);

VkBufferUsageFlags TranslateBufferUsage(BufferUsage usage);

VkFormat TranslateFormat(Format format);

VkImageUsageFlags TranslateTextureUsage(TextureUsage usage);

} // namespace lm::rhi
