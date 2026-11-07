#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

namespace lm {

class VulkanImage final {
public:
	VulkanImage() = default;
	VulkanImage(
			VkDevice device,
			VmaAllocator allocator,
			VkExtent2D extent,
			VkFormat format,
			VkImageUsageFlags usage,
			VkImageAspectFlags aspect,
			uint32_t mipLevels = 1);
	~VulkanImage();

	VulkanImage(const VulkanImage &) = delete;
	VulkanImage &operator=(const VulkanImage &) = delete;
	VulkanImage(VulkanImage &&) noexcept;
	VulkanImage &operator=(VulkanImage &&) noexcept;

	VkImage GetImage() const { return m_Image; }
	VkImageView GetImageView() const { return m_ImageView; }
	VkExtent2D GetExtent() const { return m_Extent; }
	VkFormat GetFormat() const { return m_Format; }
	uint32_t GetMipLevels() const { return m_MipLevels; }

private:
	void Destroy();

private:
	VkDevice m_Device{ VK_NULL_HANDLE };
	VmaAllocator m_Allocator{ VK_NULL_HANDLE };
	VmaAllocation m_Allocation{ VK_NULL_HANDLE };
	VkImage m_Image{ VK_NULL_HANDLE };
	VkImageView m_ImageView{ VK_NULL_HANDLE };
	VkFormat m_Format{ VK_FORMAT_UNDEFINED };
	VkExtent2D m_Extent{ 0, 0 };
	uint32_t m_MipLevels{ 0 };
};

} // namespace lm
