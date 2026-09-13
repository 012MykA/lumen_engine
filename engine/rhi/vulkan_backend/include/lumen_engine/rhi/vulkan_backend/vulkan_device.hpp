#pragma once

#include "lumen_engine/rhi/device.hpp"
#include "lumen_engine/rhi/resource_pool.hpp"
#include "lumen_engine/rhi/types.hpp"

#if defined(LUMEN_PLATFORM_WINDOWS)
	#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <optional>
#include <utility> // for std::pair
#include <vector>

namespace lm::rhi {

struct VulkanBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize size = 0;
	void *mapped = nullptr;
};

struct VulkanTexture {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkExtent2D extent;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool IsComplete() const {
		return graphicsFamily.has_value() &&
				presentFamily.has_value() &&
				computeFamily.has_value();
	}
};

class LUMEN_API VulkanDevice final : public IDevice {
private:
	static constexpr uint32_t k_VulkanApiVersion = VK_API_VERSION_1_3;
	static constexpr bool k_EnableValidation = true;

	const std::vector<const char *> k_InstanceLayers = {
		"VK_LAYER_KHRONOS_validation",
	};

	const std::vector<const char *> k_InstanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(LUMEN_PLATFORM_WINDOWS)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};
	const std::vector<const char *> k_DeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
	};

	static constexpr uint32_t k_MaxFramesInFlight = 3;

public:
	explicit VulkanDevice(void *window);
	~VulkanDevice();

	BufferHandle CreateBuffer(const BufferDesc &desc, const void *data = nullptr) override;
	void DestroyBuffer(BufferHandle handle) override;

	// Write host-visible only buffer
	void WriteBuffer(BufferHandle handle, const void *data, uint64_t size, uint64_t offset = 0) override;

	TextureHandle CreateTexture(const TextureDesc &desc, const void *data = nullptr) override;
	void DestroyTexture(TextureHandle handle) override;

	void *GetVkInstance() const override { return m_Instance; }
	void *GetVkDebugUtilsMessengerEXT() const override { return m_DebugMessenger; }
	void *GetVkSurfaceKHR() const override { return m_Surface; }
	void *GetVkPhysicalDevice() const override { return m_PhysicalDevice; }
	void *GetQueueFamilyIndices() const override { return (void *)&m_QueueFamilyIndices; }
	void *GetVkDevice() const override { return m_Device; }
	void *GetVkCommandPool() const override { return m_CommandPool; }
	void *GetVkBuffer(BufferHandle handle) const override;

private:
	// Instance
	VkInstance m_Instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

	// Device
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	QueueFamilyIndices m_QueueFamilyIndices;
	VkDevice m_Device = VK_NULL_HANDLE;

	// Queues
	VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
	VkQueue m_PresentQueue = VK_NULL_HANDLE;
	VkQueue m_ComputeQueue = VK_NULL_HANDLE;

	VkCommandPool m_CommandPool = VK_NULL_HANDLE;

	ResourcePool<VulkanBuffer, BufferTag> m_BufferPool;
	ResourcePool<VulkanTexture, TextureTag> m_TexturePool;

private:
	// --- Init ---
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface(void *window);
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateCommandPool();

	// --- Shutdown ---
	void DestroyInstance();
	void DestroyDebugMessenger();
	void DestroySurface();
	void DestroyLogicalDevice();
	void DestroyCommandPool();

private:
	// Helper functions

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	void SingleTimeQueueSubmit(VkQueue queue, VkSubmitInfo submitInfo);

	template <typename F>
		requires std::invocable<F, VkCommandBuffer>
	void SubmitSingleTimeCommand(F &&func);

	// --- Buffer ---
	std::pair<VkBuffer, VkDeviceMemory> CreateVkBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties);
	void DestroyVulkanBuffer(VulkanBuffer &buf);
	void CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size);
	void UploadViaStagingBuffer(VkBuffer dst, const void *data, VkDeviceSize size);

	// --- Enumerate ---
	std::vector<VkLayerProperties> EnumerateInstanceLayers() const;
	std::vector<VkExtensionProperties> EnumerateInstanceExtensions() const;
	std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;
	std::vector<VkExtensionProperties> EnumerateDeviceExtensions(VkPhysicalDevice device) const;

	// --- Filter ---
	std::vector<const char *> FilterSupportedDeviceExtensions(VkPhysicalDevice device, const std::vector<const char *> &wanted, bool required) const;
	std::vector<const char *> FilterSupportedInstanceLayers(const std::vector<const char *> &wanted, bool required) const;
	std::vector<const char *> FilterSupportedInstanceExtensions(const std::vector<const char *> &wanted, bool required) const;
};

} // namespace lm::rhi
