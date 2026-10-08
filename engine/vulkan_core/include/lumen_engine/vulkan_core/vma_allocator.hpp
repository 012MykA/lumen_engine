#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

namespace lm::vk {

class VmaAllocatorWrapper final {
public:
	VmaAllocatorWrapper(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
	~VmaAllocatorWrapper();

	VmaAllocatorWrapper(const VmaAllocatorWrapper &) = delete;
	VmaAllocatorWrapper &operator=(const VmaAllocatorWrapper &) = delete;
	VmaAllocatorWrapper(VmaAllocatorWrapper &&other) noexcept;
	VmaAllocatorWrapper &operator=(VmaAllocatorWrapper &&other) noexcept;

	VmaAllocator Handle() const { return m_Allocator; }
	operator VmaAllocator() const { return m_Allocator; }

private:
	VmaAllocator m_Allocator = nullptr;
};

} // namespace lm::vk
