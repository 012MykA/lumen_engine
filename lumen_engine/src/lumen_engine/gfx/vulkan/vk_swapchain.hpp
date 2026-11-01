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

private:
	void CreateSwapchain();
	void CreateImageViews();

private:
	const VulkanContext &m_Ctx;
	VkDevice m_Device{ VK_NULL_HANDLE };
	VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
	VkExtent2D m_Extent{ 0, 0 };
	VkSurfaceFormatKHR m_SurfaceFormat{};
	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;
};

} // namespace lm
