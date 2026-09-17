#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"
#include "vulkan_validation.hpp"

#include <cstring>

namespace lm::rhi {

std::pair<VkBuffer, VkDeviceMemory> VulkanRenderingDevice::CreateVkBuffer(
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

void VulkanRenderingDevice::DestroyVulkanBuffer(VulkanBuffer &buf) {
	if (buf.mapped) {
		vkUnmapMemory(m_Device, buf.memory);
	}
	vkFreeMemory(m_Device, buf.memory, nullptr);
	vkDestroyBuffer(m_Device, buf.buffer, nullptr);
}

void VulkanRenderingDevice::CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size) {
	SubmitSingleTimeCommand([&](VkCommandBuffer cmd) {
		VkBufferCopy2 region{
			.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
			.size = size,
		};
		VkCopyBufferInfo2 copyInfo{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
			.srcBuffer = src,
			.dstBuffer = dst,
			.regionCount = 1,
			.pRegions = &region,
		};
		vkCmdCopyBuffer2(cmd, &copyInfo);
	});
}

void VulkanRenderingDevice::WriteBuffer(BufferHandle handle, const void *data, uint64_t size, uint64_t offset) {
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

} // namespace lm::rhi
