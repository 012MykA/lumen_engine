#include "lumen_engine/rhi/vulkan_backend/vulkan_device.hpp"
#include "vulkan_validation.hpp"

#include <cstring>

namespace lm::rhi {

std::pair<VkBuffer, VkDeviceMemory> VulkanDevice::CreateVkBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties) {
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VkBuffer buffer;
	CHECK_VK(vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties),
	};
	VkDeviceMemory memory;
	CHECK_VK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &memory));

	CHECK_VK(vkBindBufferMemory(m_Device, buffer, memory, 0));

	return { buffer, memory };
}

void VulkanDevice::DestroyBuffer(BufferHandle handle) {
	DestroyVulkanBuffer(*m_BufferPool.Get(handle));
	m_BufferPool.Remove(handle);
}

void VulkanDevice::WriteBuffer(BufferHandle handle, const void *data, uint64_t size, uint64_t offset) {
	VulkanBuffer *buf = m_BufferPool.Get(handle);
	if (!buf) {
		throw std::runtime_error("WriteBuffer: invalid buffer handle");
	}
	if (offset + size > buf->size) {
		throw std::runtime_error("WriteBuffer: write out of buffer bounds");
	}
	if (!buf->mapped) {
		throw std::runtime_error("WriteBuffer: buffer is not host-visible (create it with MemoryUsage::HostVisible)");
	}
	std::memcpy(static_cast<uint8_t *>(buf->mapped) + offset, data, size);
}

void *VulkanDevice::GetVkBuffer(BufferHandle handle) const {
	const VulkanBuffer *buf = m_BufferPool.Get(handle);
	return buf ? buf->buffer : nullptr;
}

} // namespace lm::rhi
