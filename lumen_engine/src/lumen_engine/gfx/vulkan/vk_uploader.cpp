#include "lumen_engine/gfx/vulkan/vk_uploader.hpp"

#include <algorithm>
#include <cmath>

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

VulkanImage VulkanUploader::UploadImage(const void *pixels, uint32_t width, uint32_t height,
		VkFormat format, VmaAllocator allocator) {
	const VkDeviceSize size = static_cast<VkDeviceSize>(width) * height * 4;
	VulkanBuffer staging = VulkanBuffer::CreateStaging(pixels, size, allocator);

	const uint32_t mips = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	VulkanImage image{
		m_Device,
		allocator,
		{ width, height },
		format,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, mips
	};

	ImmediateSubmit([&](VkCommandBuffer cmd) {
		auto barrier = [&](uint32_t baseMip, uint32_t count, VkImageLayout from, VkImageLayout to,
							   VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
							   VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess) {
			VkImageMemoryBarrier2 b{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = srcStage,
				.srcAccessMask = srcAccess,
				.dstStageMask = dstStage,
				.dstAccessMask = dstAccess,
				.oldLayout = from,
				.newLayout = to,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = image.GetImage(),
				.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, baseMip, count, 0, 1 },
			};
			VkDependencyInfo dep{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &b };
			vkCmdPipelineBarrier2(cmd, &dep);
		};

		constexpr auto TRANSFER = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;

		barrier(0, mips, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_2_NONE, 0, TRANSFER, VK_ACCESS_2_TRANSFER_WRITE_BIT);

		VkBufferImageCopy2 region{
			.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
			.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
			.imageExtent = { width, height, 1 },
		};
		VkCopyBufferToImageInfo2 copy{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
			.srcBuffer = staging.GetVkBuffer(),
			.dstImage = image.GetImage(),
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = 1,
			.pRegions = &region,
		};
		vkCmdCopyBufferToImage2(cmd, &copy);

		int32_t w = static_cast<int32_t>(width), h = static_cast<int32_t>(height);
		for (uint32_t i = 1; i < mips; ++i) {
			barrier(i - 1, 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					TRANSFER, VK_ACCESS_2_TRANSFER_WRITE_BIT, TRANSFER, VK_ACCESS_2_TRANSFER_READ_BIT);

			const int32_t nw = std::max(w / 2, 1), nh = std::max(h / 2, 1);
			VkImageBlit2 blit{
				.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
				.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, 1 },
				.srcOffsets = { { 0, 0, 0 }, { w, h, 1 } },
				.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1 },
				.dstOffsets = { { 0, 0, 0 }, { nw, nh, 1 } },
			};
			VkBlitImageInfo2 blitInfo{
				.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
				.srcImage = image.GetImage(),
				.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.dstImage = image.GetImage(),
				.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.regionCount = 1,
				.pRegions = &blit,
				.filter = VK_FILTER_LINEAR,
			};
			vkCmdBlitImage2(cmd, &blitInfo);

			barrier(i - 1, 1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					TRANSFER, VK_ACCESS_2_TRANSFER_READ_BIT,
					VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
			w = nw;
			h = nh;
		}
		barrier(mips - 1, 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				TRANSFER, VK_ACCESS_2_TRANSFER_WRITE_BIT,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
	});
	return image;
}

} // namespace lm
