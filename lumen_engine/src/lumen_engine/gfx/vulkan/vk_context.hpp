#pragma once

#include "lumen_engine/gfx/vulkan/queue_family_indices.hpp"
#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <span>

namespace lm {

struct Window;

class VulkanContext final {
public:
	explicit VulkanContext(const Window &window, bool enableValidation = true);
	~VulkanContext();

	VulkanContext(const VulkanContext &) = delete;
	VulkanContext &operator=(const VulkanContext &) = delete;

private:
	void CreateInstance(bool enableValidation);
	void SetupDebugMessenger();
	void CreateSurface(const lm::Window &window);
	void PickPhysicalDevice();
	void CreateDevice();

private:
	void CheckExtensionsSupport(std::span<const char *> extensions) const;
	void CheckLayersSupport(std::span<const char *> layers) const;

	bool IsDeviceSuitable(VkPhysicalDevice device) const;

private:
	VkInstance m_Instance{ VK_NULL_HANDLE };
	VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
	VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };

	VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
	QueueFamilyIndices m_QueueFamilyIndices{};
	VkDevice m_Device{ VK_NULL_HANDLE };

	VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
	VkQueue m_PresentQueue{ VK_NULL_HANDLE };
	VkQueue m_ComputeQueue{ VK_NULL_HANDLE };
};

} // namespace lm
