#include "lumen_engine/rhi/vulkan/vulkan_device.hpp"

#include "lumen_engine/io/log.hpp"
#include "vulkan_validation.hpp"

#include <cstring>
#include <tuple>

#ifdef max
	#undef max
#endif

namespace lm::rhi {

namespace {

constexpr uint64_t noTimeout = std::numeric_limits<uint64_t>::max();

}

VulkanDevice::VulkanDevice(void *window) {
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

VulkanDevice::~VulkanDevice() {
	CHECK_VK(vkDeviceWaitIdle(m_Device));

	m_BufferPool.ForEach([this](BufferHandle handle, VulkanBuffer &buf) { DestroyVulkanBuffer(buf); });

	DestroyCommandPool();
	DestroyLogicalDevice();
	DestroySurface();
	if (m_DebugMessenger != VK_NULL_HANDLE) {
		DestroyDebugMessenger();
	}
	DestroyInstance();
}

BufferHandle VulkanDevice::CreateBuffer(const BufferDesc &desc, const void *data) {
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
		UploadViaStagingBuffer(buf.buffer, data, buf.size);
	}

	LUMEN_CORE_INFO("Buffer created:");
	LUMEN_CORE_INFO("\tsize: {}");
	LUMEN_CORE_INFO("\tusage: {}", string_VkBufferUsageFlags(usage));
	LUMEN_CORE_INFO("\tmemProps: {}", string_VkMemoryPropertyFlags(memProps));

	return m_BufferPool.Insert(buf);
}

void VulkanDevice::CreateCommandPool() {
	VkCommandPoolCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily.value(),
	};
	CHECK_VK(vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CommandPool));
}

// --- Shutdown ---

void VulkanDevice::DestroyCommandPool() {
	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
}

VkMemoryPropertyFlags VulkanDevice::TranslateMemoryUsage(MemoryUsage usage) {
	// clang-format off
	switch (usage) {
		case MemoryUsage::DeviceLocal:	return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		case MemoryUsage::HostVisible:	return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	// clang-format on
	throw std::runtime_error("TranslateMemoryUsage(): Unknown MemoryUsage");
}

VkBufferUsageFlags VulkanDevice::TranslateBufferUsage(BufferUsage usage) {
	// clang-format off
	switch (usage) {
		case BufferUsage::VertexBuffer: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case BufferUsage::IndexBuffer: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case BufferUsage::UniformBuffer: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		case BufferUsage::StorageBuffer: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
	// clang-format on
	throw std::runtime_error("TranslateBufferUsage(): Unknown BufferUsage");
}

uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type.");
}

void VulkanDevice::SingleTimeQueueSubmit(VkQueue queue, VkSubmitInfo submitInfo) {
	VkFenceCreateInfo fenceInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	VkFence copyFinishedFence;
	CHECK_VK(vkCreateFence(m_Device, &fenceInfo, nullptr, &copyFinishedFence));

	CHECK_VK(vkQueueSubmit(queue, 1, &submitInfo, copyFinishedFence));

	CHECK_VK(vkWaitForFences(m_Device, 1, &copyFinishedFence, VK_TRUE, noTimeout));

	vkDestroyFence(m_Device, copyFinishedFence, nullptr);
}

void VulkanDevice::DestroyVulkanBuffer(VulkanBuffer &buf) {
	if (buf.mapped) {
		vkUnmapMemory(m_Device, buf.memory);
	}
	vkFreeMemory(m_Device, buf.memory, nullptr);
	vkDestroyBuffer(m_Device, buf.buffer, nullptr);
}

void VulkanDevice::CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size) {
	SubmitSingleTimeCommand([&](VkCommandBuffer cmd) {
		VkBufferCopy2 region{
			.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
			.size = size,
		};
		VkCopyBufferInfo2 copyInfo{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
			.srcBuffer = src,
			.dstBuffer = dst,
			.regionCount = 1,
			.pRegions = &region,
		};
		vkCmdCopyBuffer2(cmd, &copyInfo);
	});
}

void VulkanDevice::UploadViaStagingBuffer(VkBuffer dst, const void *data, VkDeviceSize size) {
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

template <typename F>
	requires std::invocable<F, VkCommandBuffer>
inline void VulkanDevice::SubmitSingleTimeCommand(F &&func) {
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
