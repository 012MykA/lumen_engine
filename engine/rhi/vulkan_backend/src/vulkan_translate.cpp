#include "vulkan_translate.hpp"

#include <stdexcept>

namespace lm::rhi {

VkMemoryPropertyFlags TranslateMemoryUsage(MemoryUsage usage) {
	// clang-format off
	switch (usage) {
		case MemoryUsage::DeviceLocal:	return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		case MemoryUsage::HostVisible:	return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	// clang-format on
	throw std::runtime_error("TranslateMemoryUsage(): Unknown MemoryUsage");
}

VkBufferUsageFlags TranslateBufferUsage(BufferUsage usage) {
	VkBufferUsageFlags flags = 0;
	// clang-format off
	if (HasUsage(BufferUsage::VertexBuffer,  usage))		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if (HasUsage(BufferUsage::IndexBuffer,   usage))		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (HasUsage(BufferUsage::UniformBuffer, usage))		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (HasUsage(BufferUsage::StorageBuffer, usage))		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	// clang-format on
	return flags;
}

VkFormat TranslateTextureFormat(TextureFormat format) {
	// clang-format off
	switch (format) {
		case TextureFormat::RGBA8_UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGBA8_SRGB:  return VK_FORMAT_R8G8B8A8_SRGB;
		case TextureFormat::Depth32F:    return VK_FORMAT_D32_SFLOAT;
	}
	// clang-format on
	throw std::runtime_error("TranslateTextureFormat(): Unknown TextureFormat");
}

VkImageUsageFlags TranslateTextureUsage(TextureUsage usage) {
	VkImageUsageFlags result = 0;
	// clang-format off
	if (HasUsage(usage, TextureUsage::Sampled))                result |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (HasUsage(usage, TextureUsage::ColorAttachment))        result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (HasUsage(usage, TextureUsage::DepthStencilAttachment)) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	// clang-format on
	return result;
}

} // namespace lm::rhi
