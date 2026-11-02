#include "lumen_engine/gfx/vulkan/vk_buffer.hpp"

namespace lm {

VulkanBuffer::VulkanBuffer(
		VmaAllocator allocator,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VmaAllocationCreateFlags flags) :
		m_Allocator{ allocator },
		m_Size{ size } {
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VmaAllocationCreateInfo allocInfo{
		.flags = flags,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};
	VmaAllocationInfo info{};
	CHECK_VK(vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &m_Buffer, &m_Allocation, &info));
	m_Mapped = info.pMappedData;
}

VulkanBuffer::~VulkanBuffer() {
	Destroy();
}

VulkanBuffer::VulkanBuffer(VulkanBuffer &&o) noexcept :
		m_Allocator{ std::exchange(o.m_Allocator, VK_NULL_HANDLE) },
		m_Buffer{ std::exchange(o.m_Buffer, VK_NULL_HANDLE) },
		m_Allocation{ std::exchange(o.m_Allocation, VK_NULL_HANDLE) },
		m_Size{ std::exchange(o.m_Size, 0) },
		m_Mapped{ std::exchange(o.m_Mapped, nullptr) } {}

VulkanBuffer &VulkanBuffer::operator=(VulkanBuffer &&o) noexcept {
	if (this != &o) {
		Destroy();
		m_Allocator = std::exchange(o.m_Allocator, VK_NULL_HANDLE);
		m_Buffer = std::exchange(o.m_Buffer, VK_NULL_HANDLE);
		m_Allocation = std::exchange(o.m_Allocation, VK_NULL_HANDLE);
		m_Size = std::exchange(o.m_Size, 0);
		m_Mapped = std::exchange(o.m_Mapped, nullptr);
	}
	return *this;
}

VulkanBuffer VulkanBuffer::CreateStaging(VmaAllocator allocator, const void *data, VkDeviceSize size) {
	VulkanBuffer staging{ allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT };
	std::memcpy(staging.m_Mapped, data, size);
	CHECK_VK(vmaFlushAllocation(allocator, staging.m_Allocation, 0, VK_WHOLE_SIZE));
	return staging;
}

void VulkanBuffer::Destroy() {
	if (m_Buffer != VK_NULL_HANDLE) {
		vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
	}
	m_Buffer = VK_NULL_HANDLE;
	m_Allocation = VK_NULL_HANDLE;
	m_Mapped = nullptr;
}

} // namespace lm
