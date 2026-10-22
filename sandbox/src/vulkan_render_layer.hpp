#pragma once

#include "check_vk.hpp"

#include <lumen_engine/lumen_engine.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <volk.h>

#include <array>
#include <concepts>
#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription() {
		return { .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
		return {
			VkVertexInputAttributeDescription{ .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, pos) },
			VkVertexInputAttributeDescription{ .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color) },
			VkVertexInputAttributeDescription{ .location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, texCoord) },
		};
	}
};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool IsComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
	}
};

struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanRenderLayer final : public lm::Layer {
public:
	explicit VulkanRenderLayer(const lm::Window &window);

	void OnAttach() override;
	void OnDetach() override;
	void OnEvent(lm::Event &e) override;
	void OnRender() override;

private:
	// --- Requirements ---
	static constexpr uint32_t k_RequiredVulkanVersion = VK_API_VERSION_1_3;
	static constexpr bool k_EnableValidation = true;

	const std::vector<const char *> k_RequiredInstanceLayers = {
		"VK_LAYER_KHRONOS_validation",

	};
	const std::vector<const char *> k_RequiredInstanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};
	const std::vector<const char *> k_RequiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
	};

	static constexpr uint32_t k_MaxFramesInFlight = 3;

	// --- Runtime ---
	bool OnWindowResize(lm::WindowExtent extent);

	void UpdateUniformBuffer();

	// --- Init ---
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateDevice();
	void CreateSwapchain();
	void CreateImageViews();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();

	void CreateCommandPool();

	void CreateDepthResources();

	void CreateTextureImage();
	void CreateTextureImageView();
	void CreateTextureSampler();

	void CreateVertexBuffer();
	void CreateIndexBuffer();

	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void CreateCommandBuffers();
	void CreateSyncObjects();

	// --- Shutdown ---
	void Cleanup();

	// --- Query available ---
	std::vector<VkLayerProperties> EnumerateInstanceLayers() const;
	std::vector<VkExtensionProperties> EnumerateInstanceExtensions() const;
	std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;
	std::vector<VkExtensionProperties> EnumerateDeviceExtensions(VkPhysicalDevice device) const;

	// --- Filter ---
	std::vector<const char *> FilterSupportedInstanceLayers(const std::vector<const char *> &wanted, bool required) const;
	std::vector<const char *> FilterSupportedInstanceExtensions(const std::vector<const char *> &wanted, bool required) const;
	std::vector<const char *> FilterSupportedDeviceExtensions(VkPhysicalDevice device, const std::vector<const char *> &wanted, bool required) const;

	// --- Device suitability ---
	struct DeviceFeatureChain {
		VkPhysicalDeviceVulkan11Features vulkan11Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		VkPhysicalDeviceVulkan13Features vulkan13Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
		VkPhysicalDeviceFeatures2 features2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };

		void Link() {
			features2.pNext = &extendedDynamicStateFeatures;
			extendedDynamicStateFeatures.pNext = &vulkan13Features;
			vulkan13Features.pNext = &vulkan11Features;
			vulkan11Features.pNext = nullptr;
		}
	};

	void QueryDeviceFeatures(VkPhysicalDevice device, DeviceFeatureChain &outChain) const;
	bool SupportsRequiredDeviceExtensions(VkPhysicalDevice device) const;
	bool IsDeviceSuitable(VkPhysicalDevice device) const;
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;

	// Swapchain
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const;
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const;
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
	uint32_t ChooseSwapMinImageCount(const VkSurfaceCapabilitiesKHR &capabilities) const;

	void RecreateSwapchain();
	void CleanupSwapchain();

	// Shader
	static std::vector<uint32_t> readSpirV(const std::string &filename);
	[[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<uint32_t> &code) const;

	// Command buffer
	void RecordCommandBuffer(uint32_t imageIndex);

	static void TransitionImageLayout(
			VkCommandBuffer cmd,
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			VkAccessFlags2 srcAccessMasks,
			VkAccessFlags2 dstAccessMask,
			VkPipelineStageFlags2 srcStageMask,
			VkPipelineStageFlags2 dstStageMask,
			VkImageAspectFlags aspectFlags);

	// Memory
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	// Buffer
	[[nodiscard]] std::pair<VkBuffer, VkDeviceMemory> CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size);

	// Image
	[[nodiscard]] std::pair<VkImage, VkDeviceMemory> CreateImage(uint32_t width, uint32_t height,
			VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

	[[nodiscard]] VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void CopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

	// Single commands
	void SingleTimeQueueSubmit(VkQueue queue, VkSubmitInfo submitInfo);

	template <typename F>
		requires std::invocable<F, VkCommandBuffer>
	void SubmitSingleTimeCommand(F &&func);

	VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	// Depth
	VkFormat FindDepthFormat();

private:
	const lm::Window &m_Window;

	struct Context {
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		QueueFamilyIndices queueFamilyIndices;
		VkDevice device = VK_NULL_HANDLE;

		VkQueue graphicsQueue = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;
		VkQueue computeQueue = VK_NULL_HANDLE;

		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkExtent2D swapchainExtent = { 0, 0 };
		VkSurfaceFormatKHR swapchainSurfaceFormat = { .format = VK_FORMAT_UNDEFINED, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;

		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
		VkPipeline graphicsPipeline = VK_NULL_HANDLE;

		VkCommandPool commandPool = VK_NULL_HANDLE;

		VkFormat depthFormat = VK_FORMAT_UNDEFINED;
		VkImage depthImage = VK_NULL_HANDLE;
		VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
		VkImageView depthImageView = VK_NULL_HANDLE;

		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkImageView textureImageView = VK_NULL_HANDLE;
		VkSampler textureImageSampler = VK_NULL_HANDLE;

		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void *> uniformBuffersMapped;

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> presentCompleteSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
	} m_Context;
	uint32_t m_FrameIndex = 0;

	bool m_FramebufferResized = false;
};

template <typename F>
	requires std::invocable<F, VkCommandBuffer>
inline void VulkanRenderLayer::SubmitSingleTimeCommand(F &&func) {
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_Context.commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	VkCommandBuffer cmd;
	CHECK_VK(vkAllocateCommandBuffers(m_Context.device, &allocInfo, &cmd));

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

	SingleTimeQueueSubmit(m_Context.graphicsQueue, submitInfo);
	vkFreeCommandBuffers(m_Context.device, m_Context.commandPool, 1, &cmd);
}
