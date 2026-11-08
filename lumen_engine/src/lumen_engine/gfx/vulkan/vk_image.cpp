#include "lumen_engine/gfx/vulkan/vk_image.hpp"

#include "lumen_engine/core/log.hpp"

#include <utility>

namespace lm {

VulkanImage::VulkanImage(
		VkDevice device,
		VmaAllocator allocator,
		VkExtent2D extent,
		VkFormat format,
		VkImageUsageFlags usage,
		VkImageAspectFlags aspect,
		uint32_t mipLevels) :
		m_Device{ device },
		m_Allocator{ allocator },
		m_Format{ format },
		m_Extent{ extent },
		m_MipLevels{ mipLevels } {
	VkImageCreateInfo imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = m_Format,
		.extent = { m_Extent.width, m_Extent.height, 1 },
		.mipLevels = m_MipLevels,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VmaAllocationCreateInfo allocInfo{
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};
	VmaAllocationInfo info{};
	CHECK_VK(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, &m_Image, &m_Allocation, &info));

	VkImageViewCreateInfo viewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = m_Image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = { aspect, 0, m_MipLevels, 0, 1 },
	};
	const VkResult res = vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView);
	if (res != VK_SUCCESS) {
		vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
		m_Image = VK_NULL_HANDLE;
		CHECK_VK(res);
	}
}

VulkanImage::~VulkanImage() {
	Destroy();
}

VulkanImage::VulkanImage(VulkanImage &&other) noexcept :
		m_Device{ other.m_Device },
		m_Allocator{ other.m_Allocator },
		m_Image{ std::exchange(other.m_Image, VK_NULL_HANDLE) },
		m_Allocation{ std::exchange(other.m_Allocation, VK_NULL_HANDLE) },
		m_ImageView{ std::exchange(other.m_ImageView, VK_NULL_HANDLE) },
		m_Format{ other.m_Format },
		m_Extent{ other.m_Extent } {}

VulkanImage &VulkanImage::operator=(VulkanImage &&other) noexcept {
	if (this != &other) {
		Destroy();
		m_Device = other.m_Device;
		m_Allocator = other.m_Allocator;
		m_Image = std::exchange(other.m_Image, VK_NULL_HANDLE);
		m_Allocation = std::exchange(other.m_Allocation, VK_NULL_HANDLE);
		m_ImageView = std::exchange(other.m_ImageView, VK_NULL_HANDLE);
		m_Format = other.m_Format;
		m_Extent = other.m_Extent;
	}
	return *this;
}

void VulkanImage::Destroy() {
	if (m_ImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(m_Device, m_ImageView, nullptr);
		m_ImageView = VK_NULL_HANDLE;
	}
	if (m_Image != VK_NULL_HANDLE) {
		vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
		m_Image = VK_NULL_HANDLE;
		m_Allocation = VK_NULL_HANDLE;
	}
}

} // namespace lm
