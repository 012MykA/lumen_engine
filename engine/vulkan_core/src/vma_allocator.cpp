#include "lumen_engine/vulkan_core/vma_allocator.hpp"

#include "lumen_engine/vulkan_core/check_vk.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <utility>

namespace lm::vk {

VmaAllocatorWrapper::VmaAllocatorWrapper(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
	VmaAllocatorCreateInfo createInfo{
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = physicalDevice,
		.device = device,
		.instance = instance,
		.vulkanApiVersion = VK_API_VERSION_1_3,
	};
	CHECK_VK(vmaCreateAllocator(&createInfo, &m_Allocator));
}

VmaAllocatorWrapper::~VmaAllocatorWrapper() {
	if (m_Allocator) {
		vmaDestroyAllocator(m_Allocator);
	}
}

VmaAllocatorWrapper::VmaAllocatorWrapper(VmaAllocatorWrapper &&other) noexcept
		: m_Allocator(std::exchange(other.m_Allocator, nullptr)) {}

VmaAllocatorWrapper &VmaAllocatorWrapper::operator=(VmaAllocatorWrapper &&other) noexcept {
	if (this != &other) {
		if (m_Allocator) {
			vmaDestroyAllocator(m_Allocator);
		}
		m_Allocator = std::exchange(other.m_Allocator, nullptr);
	}
	return *this;
}

} // namespace lm::vk
