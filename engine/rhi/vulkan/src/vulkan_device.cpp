#include "lumen_engine/rhi/vulkan/vulkan_device.hpp"

#include "lumen_engine/io/log.hpp"
#include "vulkan_validation.hpp"

#include <cstring>
#include <limits>
#include <set>
#include <stdexcept>
#include <tuple>

#ifdef max
	#undef max
#endif

namespace lm::rhi {

namespace {

constexpr auto noTimeout = std::numeric_limits<uint64_t>::max();

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	return func ? func(instance, pCreateInfo, pAllocator, pDebugMessenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) {
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
	auto msg = std::format("\n\tVulkan Validation [{}]: {}", string_VkDebugUtilsMessageTypeFlagsEXT(type), pCallbackData->pMessage);
	// clang-format off
	switch (severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:	LUMEN_CORE_TRACE("{}", msg);	break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:		LUMEN_CORE_INFO("{}", msg);		break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:	LUMEN_CORE_WARN("{}", msg);		break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:		LUMEN_CORE_ERROR("{}", msg);	break;
		default: LUMEN_CORE_TRACE("Unknown {}", msg); break;
	}
	// clang-format on
	return VK_FALSE;
}

} // namespace

VulkanDevice::VulkanDevice(void *window) {
	LUMEN_CORE_INFO("[RHI] Initializing Vulkan backend...");

	CreateInstance();
	if (k_EnableValidation) {
		SetupDebugMessenger();
	}
	CreateSurface(window);
	PickPhysicalDevice();
	CreateDevice();
	CreateCommandPool();

	LUMEN_CORE_INFO("[RHI] IDevice initialized successfully");
	LUMEN_CORE_INFO("[RHI] Backend API: Vulkan");
}

VulkanDevice::~VulkanDevice() {
	CHECK_VK(vkDeviceWaitIdle(m_Device));

	m_BufferPool.ForEach([this](BufferHandle handle, VulkanBuffer &buf) { DestroyVulkanBuffer(buf); });

	DestroyCommandPool();
	DestroyDevice();
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

	return m_BufferPool.Insert(buf);
}

// --- Init ---

void VulkanDevice::CreateDevice() {
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		m_QueueFamilyIndices.graphicsFamily.value(),
		m_QueueFamilyIndices.presentFamily.value(),
		m_QueueFamilyIndices.computeFamily.value(),
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
		});
	}

	auto enabledExtensions = FilterSupportedDeviceExtensions(m_PhysicalDevice, k_DeviceExtensions, /*required=*/true);

	VkPhysicalDeviceVulkan11Features vulkan11Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
	VkPhysicalDeviceVulkan13Features vulkan13Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &vulkan11Features,
	};
	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
		.pNext = &vulkan13Features,
	};
	VkPhysicalDeviceFeatures2 features2{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &extendedDynamicStateFeatures,
	};
	vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);
	features2.features.samplerAnisotropy = VK_TRUE;
	vulkan11Features.shaderDrawParameters = VK_TRUE;
	vulkan13Features.dynamicRendering = VK_TRUE;
	extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features2,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
		.pEnabledFeatures = nullptr,
	};

	CHECK_VK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

	vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.presentFamily.value(), 0, &m_PresentQueue);
	vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.computeFamily.value(), 0, &m_ComputeQueue);
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

void VulkanDevice::DestroyInstance() {
	vkDestroyInstance(m_Instance, nullptr);
}

void VulkanDevice::DestroyDebugMessenger() {
	DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
}

void VulkanDevice::DestroySurface() {
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
}

void VulkanDevice::DestroyDevice() {
	vkDestroyDevice(m_Device, nullptr);
}

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
