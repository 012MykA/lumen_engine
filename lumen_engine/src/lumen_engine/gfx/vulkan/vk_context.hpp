#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"
#include "lumen_engine/gfx/vulkan/vk_queue_family_indices.hpp"

#include <span>

namespace lm {

struct Window;

class VulkanContext final {
public:
	explicit VulkanContext(const Window &window);
	~VulkanContext();

	VulkanContext(const VulkanContext &) = delete;
	VulkanContext &operator=(const VulkanContext &) = delete;

	const lm::Window &GetWindow() const { return m_Window; }
	VkSurfaceKHR GetVkSurface() const { return m_Surface; }
	VkPhysicalDevice GetVkPhysicalDevice() const { return m_PhysicalDevice; }
	const QueueFamilyIndices &GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }
	VkDevice GetVkDevice() const { return m_Device; }

private:
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateDevice();
	void CreateAllocator();

private:
	void CheckExtensionsSupport(std::span<const char *> extensions) const;
	void CheckLayersSupport(std::span<const char *> layers) const;

	bool IsDeviceSuitable(VkPhysicalDevice device) const;

private:
	const lm::Window &m_Window;

	VkInstance m_Instance{ VK_NULL_HANDLE };
	VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
	VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };

	VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
	QueueFamilyIndices m_QueueFamilyIndices{};
	VkDevice m_Device{ VK_NULL_HANDLE };

	VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
	VkQueue m_PresentQueue{ VK_NULL_HANDLE };
	VkQueue m_ComputeQueue{ VK_NULL_HANDLE };

	VmaAllocator m_Allocator{ VK_NULL_HANDLE };
};

} // namespace lm
