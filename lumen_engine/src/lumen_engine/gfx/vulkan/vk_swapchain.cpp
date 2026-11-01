#include "lumen_engine/gfx/vulkan/vk_swapchain.hpp"

#include "lumen_engine/core/window.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <span>

namespace lm {

namespace {

uint32_t ChooseMinImageCount(const VkSurfaceCapabilitiesKHR &caps);
VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &caps, const lm::Window &window);
VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::span<const VkSurfaceFormatKHR> availableFormats);
VkPresentModeKHR ChooseSwapPresentMode(std::span<const VkPresentModeKHR> availablePresentModes);

} //namespace

VulkanSwapchain::VulkanSwapchain(const VulkanContext &ctx) : m_Ctx{ ctx }, m_Device{ ctx.GetVkDevice() } {
	CreateSwapchain();
	CreateImageViews();
}

VulkanSwapchain::~VulkanSwapchain() {
	for (auto &view : m_ImageViews) {
		vkDestroyImageView(m_Device, view, nullptr);
	}
	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
}

void VulkanSwapchain::Recreate() {
	for (auto &view : m_ImageViews) {
		vkDestroyImageView(m_Device, view, nullptr);
	}
	m_ImageViews.clear();
	m_Images.clear();

	VkSwapchainKHR oldSwapchain = m_Swapchain;
	CreateSwapchain();

	vkDestroySwapchainKHR(m_Device, oldSwapchain, nullptr);
	CreateImageViews();
}

void VulkanSwapchain::CreateSwapchain() {
	const auto surface = m_Ctx.GetVkSurface();
	const auto physDevice = m_Ctx.GetVkPhysicalDevice();

	VkSurfaceCapabilitiesKHR surfaceCaps;
	CHECK_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &surfaceCaps));

	const VkExtent2D swapchainExtent = ChooseSwapExtent(surfaceCaps, m_Ctx.GetWindow());
	const uint32_t minImageCount = ChooseMinImageCount(surfaceCaps);

	uint32_t formatCount = 0;
	CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, nullptr));
	std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
	CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, availableFormats.data()));
	m_SurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

	uint32_t presentModeCount = 0;
	CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, nullptr));
	std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
	CHECK_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, availablePresentModes.data()));
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(availablePresentModes);

	const QueueFamilyIndices &indices = m_Ctx.GetQueueFamilyIndices();
	std::array<uint32_t, 2> queueFamilyIndices = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value(),
	};
	bool sameFamily = indices.graphicsFamily.value() == indices.presentFamily.value();

	VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = minImageCount,
		.imageFormat = m_SurfaceFormat.format,
		.imageColorSpace = m_SurfaceFormat.colorSpace,
		.imageExtent = swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = sameFamily ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
		.queueFamilyIndexCount = sameFamily ? 0 : static_cast<uint32_t>(queueFamilyIndices.size()),
		.pQueueFamilyIndices = sameFamily ? nullptr : queueFamilyIndices.data(),
		.preTransform = surfaceCaps.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = m_Swapchain,
	};

	CHECK_VK(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain));

	assert(m_Images.empty());
	uint32_t imageCount = 0;
	CHECK_VK(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr));
	m_Images.resize(imageCount);
	CHECK_VK(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_Images.data()));
}

void VulkanSwapchain::CreateImageViews() {
	assert(m_ImageViews.empty());

	size_t imageCount = m_Images.size();
	m_ImageViews.resize(imageCount);
	for (size_t i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_Images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_SurfaceFormat.format,
			.subresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
			},
		};
		CHECK_VK(vkCreateImageView(m_Device, &imageInfo, nullptr, &m_ImageViews[i]));
	}
}

namespace {

uint32_t ChooseMinImageCount(const VkSurfaceCapabilitiesKHR &caps) {
	uint32_t minImageCount = std::max(caps.minImageCount, 3u);
	if ((caps.maxImageCount > 0) && (minImageCount > caps.maxImageCount)) {
		minImageCount = caps.maxImageCount;
	}
	return minImageCount;
}

VkExtent2D ChooseSwapExtent(
		const VkSurfaceCapabilitiesKHR &caps,
		const lm::Window &window) {
	if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return caps.currentExtent;
	}
	lm::WindowExtent extent = window.GetExtent();
	return {
		std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width),
		std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height),
	};
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
		std::span<const VkSurfaceFormatKHR> availableFormats) {
	const auto formatIt = std::ranges::find_if(availableFormats, [](const VkSurfaceFormatKHR fmt) {
		return fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	});
	return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(
		std::span<const VkPresentModeKHR> availablePresentModes) {
	assert(std::ranges::any_of(availablePresentModes, [](const VkPresentModeKHR &mode) { return mode == VK_PRESENT_MODE_FIFO_KHR; }));
	return std::ranges::any_of(availablePresentModes, [](const VkPresentModeKHR &mode) { return mode == VK_PRESENT_MODE_MAILBOX_KHR; })
			? VK_PRESENT_MODE_MAILBOX_KHR
			: VK_PRESENT_MODE_FIFO_KHR;
}

} // namespace

} // namespace lm
