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

	m_BufferPool.ForEach([&](BufferHandle handle, VulkanBuffer &buf) {
		if (buf.mapped) {
			vkUnmapMemory(m_Device, buf.memory);
		}
		vkFreeMemory(m_Device, buf.memory, nullptr);
		vkDestroyBuffer(m_Device, buf.buffer, nullptr);
	});

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

void VulkanDevice::DestroyBuffer(BufferHandle handle) {
	m_BufferPool.Remove(handle);
}

// --- Init ---

void VulkanDevice::CreateInstance() {
	constexpr VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "No name",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Lumen Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = k_VulkanApiVersion,
	};

	const std::vector<const char *> enabledLayers = FilterSupportedInstanceLayers(k_InstanceLayers, /*required=*/true);
	const std::vector<const char *> enabledExtensions = FilterSupportedInstanceExtensions(k_InstanceExtensions, /*required=*/true);

	const VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
		.ppEnabledLayerNames = enabledLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data(),
	};

	CHECK_VK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
}

void VulkanDevice::SetupDebugMessenger() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugCallback,
	};
	CHECK_VK(CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger));
}

void VulkanDevice::CreateSurface(void *window) {
#if defined(LUMEN_PLATFORM_WINDOWS)
	HWND handle = static_cast<HWND>(window);
	const VkWin32SurfaceCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hwnd = handle,
	};
	CHECK_VK(vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface));
#else
	#error Unsupported platform
#endif
}

void VulkanDevice::PickPhysicalDevice() {
	auto devices = EnumeratePhysicalDevices();
	if (devices.empty()) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support.");
	}

	auto it = std::ranges::find_if(devices, [this](VkPhysicalDevice d) { return IsDeviceSuitable(d); });
	if (it == devices.end()) {
		throw std::runtime_error("Failed to find a suitable GPU.");
	}
	m_PhysicalDevice = *it;
	m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);
}

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

bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device) const {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(device, &props);
	bool supportsVersion = props.apiVersion >= k_VulkanApiVersion;

	auto available = EnumerateDeviceExtensions(device);
	bool supportsDeviceExtensions = std::ranges::all_of(k_DeviceExtensions, [&](const char *name) {
		return std::ranges::any_of(available, [name](const auto &p) { return std::strcmp(p.extensionName, name) == 0; });
	});

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
	vkGetPhysicalDeviceFeatures2(device, &features2);
	bool supportsFeatures =
			features2.features.samplerAnisotropy &&
			vulkan11Features.shaderDrawParameters &&
			vulkan13Features.dynamicRendering &&
			extendedDynamicStateFeatures.extendedDynamicState;

	return supportsVersion && supportsDeviceExtensions && supportsFeatures && FindQueueFamilies(device).IsComplete();
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

std::pair<VkBuffer, VkDeviceMemory> VulkanDevice::CreateVkBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties) {
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VkBuffer buffer;
	CHECK_VK(vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties),
	};
	VkDeviceMemory memory;
	CHECK_VK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &memory));

	CHECK_VK(vkBindBufferMemory(m_Device, buffer, memory, 0));

	return { buffer, memory };
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

VulkanDevice::QueueFamilyIndices VulkanDevice::FindQueueFamilies(VkPhysicalDevice device) const {
	QueueFamilyIndices indices;

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

	bool foundCombinedGraphicsPresent = false;
	bool foundDedicatedCompute = false;

	for (uint32_t i = 0; i < count; ++i) {
		const auto &family = families[i];
		bool hasGraphics = (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
		bool hasCompute = (family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;

		VkBool32 presentSupport = VK_FALSE;
		CHECK_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport));

		// Graphics fallback
		if (!foundCombinedGraphicsPresent && !indices.graphicsFamily.has_value() && hasGraphics) {
			indices.graphicsFamily = i;
		}

		// Present fallback
		if (!foundCombinedGraphicsPresent && presentSupport && !indices.presentFamily.has_value()) {
			indices.presentFamily = i;
		}

		// Prefered Graphics & Present
		if (!foundCombinedGraphicsPresent && hasGraphics && presentSupport) {
			indices.graphicsFamily = i;
			indices.presentFamily = i;
			foundCombinedGraphicsPresent = true;
		}

		// Compute
		if (!foundDedicatedCompute && hasCompute && !hasGraphics) {
			indices.computeFamily = i;
			foundDedicatedCompute = true;
		} else if (!foundDedicatedCompute && !indices.computeFamily.has_value() && hasCompute) {
			indices.computeFamily = i;
		}
	}

	return indices;
}

} // namespace lm::rhi
