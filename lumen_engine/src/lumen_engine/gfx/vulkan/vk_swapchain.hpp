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

	void Recreate();

private:
	void CreateSwapchain();
	void CreateImageViews();

private:
	const VulkanContext &m_Ctx;
	VkDevice m_Device{ VK_NULL_HANDLE };
	VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
	VkSurfaceFormatKHR m_SurfaceFormat;
	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;
};

} // namespace lm
