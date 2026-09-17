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

} // namespace lm::rhi
