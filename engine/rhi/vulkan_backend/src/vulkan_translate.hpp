#pragma once

#include "lumen_engine/rhi/types.hpp"

#include <vulkan/vulkan.h>

namespace lm::rhi {

VkMemoryPropertyFlags TranslateMemoryUsage(MemoryUsage usage);

VkBufferUsageFlags TranslateBufferUsage(BufferUsage usage);

VkFormat TranslateFormat(Format format);

VkImageUsageFlags TranslateTextureUsage(TextureUsage usage);

VkFilter TranslateFilter(Filter filter);

VkSamplerAddressMode TranslateAddressMode(AddressMode mode);

VkShaderStageFlags TranslateShaderStageFlags(ShaderStage stage);
VkShaderStageFlagBits TranslateShaderStageBits(ShaderStage stage);

VkFormat TranslateVertexAttribFormat(VertexAttribFormat format);

VkPrimitiveTopology TranslatePrimitiveTopology(PrimitiveTopology topology);

VkCullModeFlags TranslateCullMode(CullMode mode);

VkFrontFace TranslateFrontFace(FrontFace face);

VkCompareOp TranslateCompareOp(CompareOp op);

} // namespace lm::rhi
