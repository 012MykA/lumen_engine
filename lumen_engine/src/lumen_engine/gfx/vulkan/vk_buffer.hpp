#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

namespace lm {

class VulkanBuffer {
public:
	VulkanBuffer() = default;
	VulkanBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
			VmaAllocationCreateFlags flags = 0);
	~VulkanBuffer();

	VulkanBuffer(const VulkanBuffer &) = delete;
	VulkanBuffer &operator=(const VulkanBuffer &) = delete;
	VulkanBuffer(VulkanBuffer &&other) noexcept;
	VulkanBuffer &operator=(VulkanBuffer &&other) noexcept;

	static VulkanBuffer CreateStaging(
			VmaAllocator allocator,
			const void *data,
			VkDeviceSize size);

	VkBuffer GetVkBuffer() const { return m_Buffer; }
	VkDeviceSize GetSize() const { return m_Size; }

private:
	void Destroy();

private:
	VmaAllocator m_Allocator{ VK_NULL_HANDLE };
	VkBuffer m_Buffer{ VK_NULL_HANDLE };
	VmaAllocation m_Allocation{ VK_NULL_HANDLE };
	VkDeviceSize m_Size{ 0 };
	void *m_Mapped{ nullptr };
};

} // namespace lm
