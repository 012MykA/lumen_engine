#pragma once

#include "lumen_engine/rhi/device.hpp"
#include "lumen_engine/rhi/resource_pool.hpp"
#include "lumen_engine/rhi/types.hpp"

#if defined(LUMEN_PLATFORM_WINDOWS)
	#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
//
#include "lumen_engine/rhi/vulkan_backend/vulkan_validation.hpp"

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

	void Destroy(VkDevice device) {
		if (mapped) {
			vkUnmapMemory(device, memory);
		}
		vkFreeMemory(device, memory, nullptr);
		vkDestroyBuffer(device, buffer, nullptr);
	}
};

struct VulkanTexture {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_UNDEFINED;
	uint32_t width = 0, height = 0;
	uint32_t mipLevels = 1;

	void Destroy(VkDevice device) {
		vkDestroyImageView(device, view, nullptr);
		vkFreeMemory(device, memory, nullptr);
		vkDestroyImage(device, image, nullptr);
	}
};

struct VulkanSampler {
	VkSampler sampler = VK_NULL_HANDLE;

	void Destroy(VkDevice device) {
		vkDestroySampler(device, sampler, nullptr);
	}
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

class LUMEN_API VulkanRenderingDevice final : public IDevice {
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
	explicit VulkanRenderingDevice(void *window);
	~VulkanRenderingDevice();

	BufferHandle CreateBuffer(const BufferDesc &desc, const void *data = nullptr) override;
	void DestroyBuffer(BufferHandle handle) override;
	// Write host-visible only buffer
	void WriteBuffer(BufferHandle handle, const void *data, uint64_t size, uint64_t offset = 0) override;

	TextureHandle CreateTexture(const TextureDesc &desc, const void *data = nullptr) override;
	void DestroyTexture(TextureHandle handle) override;

	SamplerHandle CreateSampler(const SamplerDesc &desc) override;
	void DestroySampler(SamplerHandle handle) override;

	void *GetVkInstance() const override { return m_Instance; }
	void *GetVkDebugUtilsMessengerEXT() const override { return m_DebugMessenger; }
	void *GetVkSurfaceKHR() const override { return m_Surface; }
	void *GetVkPhysicalDevice() const override { return m_PhysicalDevice; }
	void *GetQueueFamilyIndices() const override { return (void *)&m_QueueFamilyIndices; }
	void *GetVkDevice() const override { return m_Device; }
	void *GetVkCommandPool() const override { return m_CommandPool; }
	void *GetVkBuffer(BufferHandle handle) const override;
	void *GetVkImage(TextureHandle handle) const override;
	void *GetVkImageView(TextureHandle handle) const override;
	void *GetVkSampler(SamplerHandle handle) const override;

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
	ResourcePool<VulkanSampler, SamplerTag> m_SamplerPool;

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

	void SingleTimeQueueSubmit(VkQueue queue, const VkSubmitInfo &submitInfo);

	template <typename F>
		requires std::invocable<F, VkCommandBuffer>
	void SubmitSingleTimeCommand(F &&func);

	// --- Buffer ---
	std::pair<VkBuffer, VkDeviceMemory> CreateVkBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties);
	void CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size);
	void UploadViaStagingBuffer(VkBuffer dst, const void *data, VkDeviceSize size);

	// --- Texture ---
	std::pair<VkImage, VkDeviceMemory> CreateVkImage(
			VkFormat format,
			VkExtent3D extent,
			uint32_t mipLevels,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties);

	VkImageView CreateVkImageView(
			VkImage image,
			VkFormat format,
			VkImageAspectFlags aspectFlags,
			uint32_t mipLevels);

	void TransitionImageLayout(
			VkCommandBuffer cmd,
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout);

	void CopyBufferToImage(
			VkCommandBuffer cmd,
			VkBuffer buffer,
			VkImage image,
			VkExtent3D extent);

	VkSampler CreateVkSampler(const VkSamplerCreateInfo &createInfo);

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

template <typename F>
	requires std::invocable<F, VkCommandBuffer>
inline void VulkanRenderingDevice::SubmitSingleTimeCommand(F &&func) {
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_CommandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	VkCommandBuffer cmd;
	CHECK_VK(vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd));

	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	CHECK_VK(vkBeginCommandBuffer(cmd, &beginInfo));

	func(cmd);

	CHECK_VK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd,
	};

	SingleTimeQueueSubmit(m_GraphicsQueue, submitInfo);
	vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &cmd);
}

} // namespace lm::rhi
