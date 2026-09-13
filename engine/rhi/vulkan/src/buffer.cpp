#include "lumen_engine/rhi/vulkan/vulkan_device.hpp"
#include "vulkan_validation.hpp"

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

} // namespace lm::rhi
