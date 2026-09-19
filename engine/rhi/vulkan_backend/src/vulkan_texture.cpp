#include "lumen_engine/logging/log.hpp"
#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"
#include "lumen_engine/rhi/vulkan_backend/vulkan_validation.hpp"
#include "vulkan_translate.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <tuple>

namespace lm::rhi {

namespace {

bool IsDepthFormat(VkFormat format) {
	switch (format) {
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return true;
		default:
			return false;
	}
}

} //namespace

TextureHandle VulkanRenderingDevice::CreateTexture(const TextureDesc &desc, const void *data) {
	if (desc.generateMipmaps && !data) {
		throw std::runtime_error("CreateTexture(): generateMipmaps requires initial data to blit from");
	}

	uint32_t mipLevels = desc.mipLevels;
	const uint32_t maxPossibleMips = static_cast<uint32_t>(std::floor(std::log2(std::max(desc.width, desc.height)))) + 1;
	if (desc.mipLevels == TextureDesc::k_AutoCalcMipLevels) {
		mipLevels = desc.generateMipmaps ? maxPossibleMips : 1;
	} else if (mipLevels > maxPossibleMips) {
		throw std::runtime_error(std::format(
				"CreateTexture(): mipLevels ({}) exceeds maximum possible for {}x{} texture ({})",
				mipLevels, desc.width, desc.height, maxPossibleMips));
	}

	if (mipLevels > 1 && !desc.generateMipmaps && data) {
		throw std::runtime_error("CreateTexture(): uploading pre-baked mip chain is not supported yet;"
								 "use generateMipmaps=true or mipLevels=1");
	}

	VulkanTexture tex{
		.format = TranslateFormat(desc.format),
		.width = desc.width,
		.height = desc.height,
		.mipLevels = mipLevels,
	};

	VkImageUsageFlags usage = TranslateTextureUsage(desc.usage);
	if (data) {
		usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (desc.generateMipmaps) {
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	std::tie(tex.image, tex.memory) = CreateVkImage(tex.format, { tex.width, tex.height, 1 },
			tex.mipLevels, usage, TranslateMemoryUsage(desc.memory));

	VkImageAspectFlags aspect = IsDepthFormat(tex.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	tex.view = CreateVkImageView(tex.image, tex.format, aspect, tex.mipLevels);

	if (data) {
		uint32_t bpp = FormatBytesPerPixel(desc.format);
		if (bpp == 0) {
			throw std::runtime_error("CreateTexture(): Format does not support data upload");
		}
		VkDeviceSize size = static_cast<VkDeviceSize>(tex.width) * tex.height * bpp;
		auto [stagingBuffer, stagingBufferMemory] = CreateVkBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void *mapped;
		CHECK_VK(vkMapMemory(m_Device, stagingBufferMemory, 0, size, 0, &mapped));
		std::memcpy(mapped, data, size);
		vkUnmapMemory(m_Device, stagingBufferMemory);

		SubmitSingleTimeCommand([&](VkCommandBuffer cmd) {
			TransitionImageLayout(cmd, tex.image,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					aspect,
					tex.mipLevels);

			CopyBufferToImage(cmd, stagingBuffer, tex.image, { tex.width, tex.height, 1 });

			if (desc.generateMipmaps && tex.mipLevels > 1) {
				GenerateMipmaps(cmd, tex.image, tex.format,
						static_cast<int32_t>(tex.width),
						static_cast<int32_t>(tex.height),
						tex.mipLevels);
			} else {
				TransitionImageLayout(cmd, tex.image,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						aspect,
						tex.mipLevels);
			}
		});

		vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
		vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
	}

	LUMEN_CORE_INFO("Texture created:");
	LUMEN_CORE_INFO("\tsize: {} bytes (mip 0)", static_cast<VkDeviceSize>(tex.width) * tex.height * FormatBytesPerPixel(desc.format));
	LUMEN_CORE_INFO("\tmipLevels: {}{}", tex.mipLevels, (desc.mipLevels == TextureDesc::k_AutoCalcMipLevels) ? "(auto)" : "");
	LUMEN_CORE_INFO("\tformat: {}", string_VkFormat(tex.format));
	LUMEN_CORE_INFO("\tusage: {}", string_VkImageUsageFlags(usage));
	LUMEN_CORE_INFO("\tmemProps: {}", string_VkMemoryPropertyFlags(TranslateMemoryUsage(desc.memory)));

	return m_TexturePool.Insert(tex);
}

void VulkanRenderingDevice::DestroyTexture(TextureHandle handle) {
	if (VulkanTexture *tex = m_TexturePool.Get(handle)) {
		tex->Destroy(m_Device);
		m_TexturePool.Remove(handle);
	}
}

void *VulkanRenderingDevice::GetVkImage(TextureHandle handle) const {
	const VulkanTexture *tex = m_TexturePool.Get(handle);
	return tex ? tex->image : nullptr;
}

void *VulkanRenderingDevice::GetVkImageView(TextureHandle handle) const {
	const VulkanTexture *tex = m_TexturePool.Get(handle);
	return tex ? tex->view : nullptr;
}

std::pair<VkImage, VkDeviceMemory> VulkanRenderingDevice::CreateVkImage(
		VkFormat format,
		VkExtent3D extent,
		uint32_t mipLevels,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties) {
	VkImageCreateInfo imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = extent,
		.mipLevels = mipLevels,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VkImage image;
	CHECK_VK(vkCreateImage(m_Device, &imageInfo, nullptr, &image));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties),
	};
	VkDeviceMemory memory;
	CHECK_VK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &memory));
	CHECK_VK(vkBindImageMemory(m_Device, image, memory, 0));

	return { image, memory };
}

VkImageView VulkanRenderingDevice::CreateVkImageView(VkImage image, VkFormat format,
		VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
	VkImageViewCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange{
				.aspectMask = aspectFlags,
				.baseMipLevel = 0,
				.levelCount = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1,
		},
	};
	VkImageView imageView;
	CHECK_VK(vkCreateImageView(m_Device, &createInfo, nullptr, &imageView));
	return imageView;
}

void VulkanRenderingDevice::TransitionImageLayout(
		VkCommandBuffer cmd,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageAspectFlags aspectFlags,
		uint32_t mipLevels) {
	VkImageMemoryBarrier2 barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = {
				.aspectMask = aspectFlags,
				.baseMipLevel = 0,
				.levelCount = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1,
		},
	};

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_2_NONE;
		barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

		barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

		barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	} else {
		throw std::invalid_argument("TransitionImageLayout(): Unsupported layout transition");
	}

	VkDependencyInfo dependencyInfo{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier,
	};
	vkCmdPipelineBarrier2(cmd, &dependencyInfo);
}

void VulkanRenderingDevice::GenerateMipmaps(
		VkCommandBuffer cmd,
		VkImage image,
		VkFormat format,
		int32_t width,
		int32_t height,
		uint32_t mipLevels) {
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProps);
	if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("GenerateMipmaps(): texture format does not support linear blitting");
	}

	VkImageMemoryBarrier2 barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
		},
	};

	int32_t mipWidth = width, mipHeight = height;
	for (uint32_t i = 1; i < mipLevels; ++i) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		VkDependencyInfo dep{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier };
		vkCmdPipelineBarrier2(cmd, &dep);

		int32_t nextWidth = mipWidth > 1 ? mipWidth / 2 : 1;
		int32_t nextHeight = mipHeight > 1 ? mipHeight / 2 : 1;

		VkImageBlit2 blit{
			.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
			.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, 1 },
			.srcOffsets = { { 0, 0, 0 }, { mipWidth, mipHeight, 1 } },
			.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1 },
			.dstOffsets = { { 0, 0, 0 }, { nextWidth, nextHeight, 1 } },
		};
		VkBlitImageInfo2 blitInfo{
			.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
			.srcImage = image,
			.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.dstImage = image,
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = 1,
			.pRegions = &blit,
			.filter = VK_FILTER_LINEAR,
		};
		vkCmdBlitImage2(cmd, &blitInfo);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		VkDependencyInfo dep2{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier };
		vkCmdPipelineBarrier2(cmd, &dep2);

		mipWidth = nextWidth;
		mipHeight = nextHeight;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	VkDependencyInfo dep3{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier };
	vkCmdPipelineBarrier2(cmd, &dep3);
}

void VulkanRenderingDevice::CopyBufferToImage(
		VkCommandBuffer cmd,
		VkBuffer buffer,
		VkImage image,
		VkExtent3D extent) {
	VkBufferImageCopy2 region{
		.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
		},
		.imageOffset = { 0, 0, 0 },
		.imageExtent = extent,
	};
	VkCopyBufferToImageInfo2 copyInfo{
		.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
		.srcBuffer = buffer,
		.dstImage = image,
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = 1,
		.pRegions = &region
	};
	vkCmdCopyBufferToImage2(cmd, &copyInfo);
}

} // namespace lm::rhi
