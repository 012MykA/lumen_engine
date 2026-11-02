#pragma once

#include "lumen_engine/gfx/vulkan/vk_buffer.hpp"
#include "lumen_engine/gfx/vulkan/vk_common.hpp"
#include "lumen_engine/gfx/vulkan/vk_context.hpp"

#include <functional>

namespace lm {

class VulkanUploader {
public:
	VulkanUploader(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex);
	~VulkanUploader();

	VulkanUploader(const VulkanUploader &) = delete;
	VulkanUploader &operator=(const VulkanUploader &) = delete;

	void ImmediateSubmit(const std::function<void(VkCommandBuffer)> &record);

	VulkanBuffer UploadBuffer(const void *data, VkDeviceSize size,
			VkBufferUsageFlags usage, VmaAllocator allocator);

private:
	VkDevice m_Device{ VK_NULL_HANDLE };
	VkQueue m_Queue{ VK_NULL_HANDLE };
	VkCommandPool m_Pool{ VK_NULL_HANDLE };
	VkCommandBuffer m_Cmd{ VK_NULL_HANDLE };
	VkFence m_Fence{ VK_NULL_HANDLE };
};

} // namespace lm
