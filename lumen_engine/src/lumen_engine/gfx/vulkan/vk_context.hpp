#pragma once

#include "lumen_engine/gfx/vulkan/queue_family_indices.hpp"
#include "lumen_engine/gfx/vulkan/vk_common.hpp"

namespace lm {

struct Window;

class VulkanContext final {
public:
	VulkanContext(const Window &window, bool enableValidation = true);
	~VulkanContext();

	VulkanContext(const VulkanContext &) = delete;
	VulkanContext &operator=(const VulkanContext &) = delete;

private:
	void CreateInstance(bool enableValidation);
	void SetupDebugMessenger();

private:
	VkInstance m_Instance{ VK_NULL_HANDLE };
	VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
	VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };

	VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
	QueueFamilyIndices m_QueueFamilyIndices{};
	VkDevice m_Device{ VK_NULL_HANDLE };

	VkQueue m_PraphicsQueue{ VK_NULL_HANDLE };
	VkQueue m_GresentQueue{ VK_NULL_HANDLE };
	VkQueue m_ComputeQueue{ VK_NULL_HANDLE };
};

} // namespace lm
