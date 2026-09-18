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

VkFormat TranslateFormat(Format format) {
	// clang-format off
	switch (format) {
		case Format::RGBA8_UNORM:			return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::RGBA8_SRGB:			return VK_FORMAT_R8G8B8A8_SRGB;
		case Format::BGRA8_UNORM:			return VK_FORMAT_B8G8R8A8_UNORM;
		case Format::BGRA8_SRGB:			return VK_FORMAT_B8G8R8A8_SRGB;
		case Format::RGBA16_SFLOAT:			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::RGBA32_SFLOAT:			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Format::D32_SFLOAT:			return VK_FORMAT_D32_SFLOAT;
		case Format::D24_UNORM_S8_UINT:		return VK_FORMAT_D24_UNORM_S8_UINT;
		case Format::D32_SFLOAT_S8_UINT:	return VK_FORMAT_D32_SFLOAT_S8_UINT;
		default:							return VK_FORMAT_UNDEFINED;
	}
	// clang-format on
}

VkImageUsageFlags TranslateTextureUsage(TextureUsage usage) {
	VkImageUsageFlags flags = 0;
	// clang-format off
	if (HasUsage(TextureUsage::Sampled, usage))					flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (HasUsage(TextureUsage::Storage, usage))					flags |= VK_IMAGE_USAGE_STORAGE_BIT;
	if (HasUsage(TextureUsage::ColorAttachment, usage))			flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (HasUsage(TextureUsage::DepthStencilAttachment, usage))	flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (HasUsage(TextureUsage::TransferSrc, usage))				flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (HasUsage(TextureUsage::TransferDst, usage))				flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	// clang-format on
	return flags;
}

} // namespace lm::rhi
