#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"

#include "lumen_engine/logging/log.hpp"
#include "vulkan_translate.hpp"

#include <cstring>
#include <tuple>

#ifdef max
	#undef max
#endif

namespace lm::rhi {

namespace {

constexpr uint64_t noTimeout = std::numeric_limits<uint64_t>::max();

}

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

	m_BufferPool.ForEach([this](BufferHandle handle, VulkanBuffer &buf) { DestroyVulkanBuffer(buf); });

	DestroyCommandPool();
	DestroyLogicalDevice();
	DestroySurface();
	if (m_DebugMessenger != VK_NULL_HANDLE) {
		DestroyDebugMessenger();
	}
	DestroyInstance();
}

BufferHandle VulkanRenderingDevice::CreateBuffer(const BufferDesc &desc, const void *data) {
	VulkanBuffer buf;
	buf.size = static_cast<VkDeviceSize>(desc.size);

	VkBufferUsageFlags usage = TranslateBufferUsage(desc.usage);
	VkMemoryPropertyFlags memProps = TranslateMemoryUsage(desc.memory);

	if (desc.memory == MemoryUsage::HostVisible) {
		std::tie(buf.buffer, buf.memory) = CreateVkBuffer(buf.size, usage, memProps);
		CHECK_VK(vkMapMemory(m_Device, buf.memory, 0, buf.size, 0, &buf.mapped));
		if (data) {
			std::memcpy(buf.mapped, data, buf.size);
		}
	} else {
		std::tie(buf.buffer, buf.memory) = CreateVkBuffer(buf.size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, memProps);
		if (data) {
			UploadViaStagingBuffer(buf.buffer, data, buf.size);
		}
	}

	LUMEN_CORE_INFO("Buffer created:");
	LUMEN_CORE_INFO("\tsize: {} bytes", buf.size);
	LUMEN_CORE_INFO("\tusage: {}", string_VkBufferUsageFlags(usage));
	LUMEN_CORE_INFO("\tmemProps: {}", string_VkMemoryPropertyFlags(memProps));

	return m_BufferPool.Insert(buf);
}

void VulkanRenderingDevice::DestroyBuffer(BufferHandle handle) {
	if (VulkanBuffer *buf = m_BufferPool.Get(handle)) {
		DestroyVulkanBuffer(*buf);
		m_BufferPool.Remove(handle);
	}
}

void *VulkanRenderingDevice::GetVkBuffer(BufferHandle handle) const {
	const VulkanBuffer *buf = m_BufferPool.Get(handle);
	return buf ? buf->buffer : nullptr;
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
