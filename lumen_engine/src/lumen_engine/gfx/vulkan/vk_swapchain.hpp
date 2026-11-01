#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"
#include "lumen_engine/gfx/vulkan/vk_context.hpp"

#include <vector>

namespace lm {

struct Window;

class VulkanSwapchain final {
public:
	VulkanSwapchain(const VulkanContext &ctx);
	~VulkanSwapchain();

	VulkanSwapchain(const VulkanSwapchain &) = delete;
	VulkanSwapchain &operator=(const VulkanSwapchain &) = delete;

	bool Recreate();

	VkSwapchainKHR GetVkSwapchain() const { return m_Swapchain; }
	VkExtent2D GetExtent() const { return m_Extent; }
	VkFormat GetFormat() const { return m_SurfaceFormat.format; }
	VkImage GetImage(uint32_t i) const { return m_Images[i]; }
	VkImageView GetImageView(uint32_t i) const { return m_ImageViews[i]; }
	VkSemaphore GetRenderFinished(uint32_t i) const { return m_RenderFinished[i]; }

private:
	void CreateSwapchain();
	void CreateImageViews();
	void CreateSyncObjects();

	void DestroyImageViews();
	void DestroySyncObjects();

private:
	const VulkanContext &m_Ctx;
	VkDevice m_Device{ VK_NULL_HANDLE };
	VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
	VkExtent2D m_Extent{ 0, 0 };
	VkSurfaceFormatKHR m_SurfaceFormat{};
	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;
	std::vector<VkSemaphore> m_RenderFinished;
};

} // namespace lm
