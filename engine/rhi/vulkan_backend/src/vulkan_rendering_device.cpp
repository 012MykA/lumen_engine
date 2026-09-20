#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"

#include "lumen_engine/logging/log.hpp"
#include "vulkan_translate.hpp"

#include <cstring>
#include <tuple>

namespace lm::rhi {

namespace {

constexpr uint64_t noTimeout = std::numeric_limits<uint64_t>::max();

} //namespace

VulkanRenderingDevice::VulkanRenderingDevice(void *window) {
	LUMEN_CORE_INFO("[RHI] Initializing Vulkan backend...");

	CreateInstance();
	if (k_EnableValidation) {
		SetupDebugMessenger();
	}
	CreateSurface(window);
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateCommandPool();

	LUMEN_CORE_INFO("[RHI] IDevice initialized successfully");
	LUMEN_CORE_INFO("[RHI] Backend API: Vulkan");
}

VulkanRenderingDevice::~VulkanRenderingDevice() {
	try {
		CHECK_VK(vkDeviceWaitIdle(m_Device));
	} catch (const std::exception &e) {
		LUMEN_CORE_CRITICAL("{}", e.what());
	}

	m_PipelinePool.ForEach([this](PipelineHandle handle, VkPipeline &pipeline) { vkDestroyPipeline(m_Device, pipeline, nullptr); });
	m_ShaderPool.ForEach([this](ShaderHandle handle, VkShaderModule &shader) { vkDestroyShaderModule(m_Device, shader, nullptr); });
	m_SamplerPool.ForEach([this](SamplerHandle handle, VkSampler &sampler) { vkDestroySampler(m_Device, sampler, nullptr); });
	m_TexturePool.ForEach([this](TextureHandle handle, VulkanTexture &tex) { tex.Destroy(m_Device); });
	m_BufferPool.ForEach([this](BufferHandle handle, VulkanBuffer &buf) { buf.Destroy(m_Device); });

	DestroyCommandPool();
	DestroyLogicalDevice();
	DestroySurface();
	if (m_DebugMessenger != VK_NULL_HANDLE) {
		DestroyDebugMessenger();
	}
	DestroyInstance();
}

void VulkanRenderingDevice::CreateCommandPool() {
	VkCommandPoolCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily.value(),
	};
	CHECK_VK(vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CommandPool));
}

// --- Shutdown ---

void VulkanRenderingDevice::DestroyCommandPool() {
	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
}

uint32_t VulkanRenderingDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type.");
}

void VulkanRenderingDevice::SingleTimeQueueSubmit(VkQueue queue, const VkSubmitInfo &submitInfo) {
	VkFenceCreateInfo fenceInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	VkFence copyFinishedFence;
	CHECK_VK(vkCreateFence(m_Device, &fenceInfo, nullptr, &copyFinishedFence));

	CHECK_VK(vkQueueSubmit(queue, 1, &submitInfo, copyFinishedFence));

	CHECK_VK(vkWaitForFences(m_Device, 1, &copyFinishedFence, VK_TRUE, noTimeout));

	vkDestroyFence(m_Device, copyFinishedFence, nullptr);
}

void VulkanRenderingDevice::UploadViaStagingBuffer(VkBuffer dst, const void *data, VkDeviceSize size) {
	auto [stagingBuffer, stagingBufferMemory] = CreateVkBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void *stagingData;
	CHECK_VK(vkMapMemory(m_Device, stagingBufferMemory, 0, size, 0, &stagingData));
	std::memcpy(stagingData, data, static_cast<size_t>(size));
	vkUnmapMemory(m_Device, stagingBufferMemory);

	CopyBuffer(dst, stagingBuffer, size);

	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
}

} // namespace lm::rhi
