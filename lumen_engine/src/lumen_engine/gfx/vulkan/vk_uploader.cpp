#include "lumen_engine/gfx/vulkan/vk_uploader.hpp"

namespace lm {

VulkanUploader::VulkanUploader(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex) :
		m_Device{ device },
		m_Queue{ queue } {
	VkCommandPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		.queueFamilyIndex = queueFamilyIndex,
	};
	CHECK_VK(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_Pool));

	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_Pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	CHECK_VK(vkAllocateCommandBuffers(m_Device, &allocInfo, &m_Cmd));

	VkFenceCreateInfo fenceInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	CHECK_VK(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Fence));
}

VulkanUploader::~VulkanUploader() {
	vkDestroyFence(m_Device, m_Fence, nullptr);
	vkDestroyCommandPool(m_Device, m_Pool, nullptr);
}

void VulkanUploader::ImmediateSubmit(const std::function<void(VkCommandBuffer)> &record) {
	CHECK_VK(vkResetCommandPool(m_Device, m_Pool, 0));

	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	CHECK_VK(vkBeginCommandBuffer(m_Cmd, &beginInfo));
	{
		record(m_Cmd);
	}
	CHECK_VK(vkEndCommandBuffer(m_Cmd));

	VkCommandBufferSubmitInfo cmdInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = m_Cmd,
	};
	VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdInfo,
	};
	CHECK_VK(vkQueueSubmit2(m_Queue, 1, &submitInfo, m_Fence));
	CHECK_VK(vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, UINT64_MAX));
	CHECK_VK(vkResetFences(m_Device, 1, &m_Fence));
}

VulkanBuffer VulkanUploader::UploadBuffer(const void *data, VkDeviceSize size,
		VkBufferUsageFlags usage, VmaAllocator allocator) {
	VulkanBuffer staging = VulkanBuffer::CreateStaging(data, size, allocator);
	VulkanBuffer gpu{ allocator, size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT };

	ImmediateSubmit([&](VkCommandBuffer cmd) {
		VkBufferCopy2 region{
			.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
			.size = size,
		};
		VkCopyBufferInfo2 copyInfo{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
			.srcBuffer = staging.GetVkBuffer(),
			.dstBuffer = gpu.GetVkBuffer(),
			.regionCount = 1,
			.pRegions = &region,
		};
		vkCmdCopyBuffer2(cmd, &copyInfo);
	});

	return gpu;
}

} // namespace lm
