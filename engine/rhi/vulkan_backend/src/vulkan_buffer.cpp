#include "lumen_engine/logging/log.hpp"
#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"
#include "lumen_engine/rhi/vulkan_backend/vulkan_validation.hpp"
#include "vulkan_translate.hpp"

#include <cstring>

namespace lm::rhi {

BufferHandle VulkanRenderingDevice::CreateBuffer(const BufferDesc &desc, const void *data) {
	VulkanBuffer buf;
	buf.size = static_cast<VkDeviceSize>(desc.size);

	VkBufferUsageFlags usage = TranslateBufferUsage(desc.usage);
	VkMemoryPropertyFlags memProps = TranslateMemoryUsage(desc.memory);

	if (desc.memory == MemoryUsage::HostVisible) {
		std::tie(buf.buffer, buf.memory) = CreateVkBuffer(buf.size, usage, memProps);
		CHECK_VK(vkMapMemory(m_Device, buf.memory, 0, buf.size, 0, &buf.mapped));
		if (data) {
			std::memcpy(buf.mapped, data, buf.size);
		}
	} else {
		std::tie(buf.buffer, buf.memory) = CreateVkBuffer(buf.size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, memProps);
		if (data) {
			UploadViaStagingBuffer(buf.buffer, data, buf.size);
		}
	}

	LUMEN_CORE_INFO("Buffer created:");
	LUMEN_CORE_INFO("\tsize: {} bytes", buf.size);
	LUMEN_CORE_INFO("\tusage: {}", string_VkBufferUsageFlags(usage));
	LUMEN_CORE_INFO("\tmemProps: {}", string_VkMemoryPropertyFlags(memProps));

	return m_BufferPool.Insert(buf);
}

void VulkanRenderingDevice::DestroyBuffer(BufferHandle handle) {
	if (VulkanBuffer *buf = m_BufferPool.Get(handle)) {
		buf->Destroy(m_Device);
		m_BufferPool.Remove(handle);
	}
}

void *VulkanRenderingDevice::GetVkBuffer(BufferHandle handle) const {
	const VulkanBuffer *buf = m_BufferPool.Get(handle);
	return buf ? buf->buffer : nullptr;
}

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

} // namespace lm::rhi
