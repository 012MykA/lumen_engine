#pragma once

#include "check_vk.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <lumen_engine/lumen_engine.hpp>

#if defined(LUMEN_PLATFORM_WINDOWS)
	#define VK_USE_PLATFORM_WIN32_KHR
#else
	#error "Unsupported platform"
#endif
#include <vulkan/vulkan.h>

#include <array>
#include <concepts>
#include <cstdint>
#include <memory>
#include <optional>
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
	explicit VulkanRenderLayer(lm::Window *window);

	void OnAttach() override;
	void OnDetach() override;
	void OnEvent(lm::Event &e) override;
	void OnRender() override;

private:
	static constexpr uint32_t k_MaxFramesInFlight = 3;

	// --- Runtime ---
	bool OnWindowResize(lm::WindowExtent extent);

	void UpdateUniformBuffer();

	// --- Init ---
	void CreateSwapchain();
	void CreateImageViews();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();

	void CreateDepthResources();

	void CreateTextureImage();

	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void CreateCommandBuffers();
	void CreateSyncObjects();

	// ImGui
	void CreateImGuiDescriptorPool();
	void InitImGui();

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

private:
	lm::Window *m_Window;
	std::unique_ptr<lm::rhi::IDevice> m_Device;

	lm::rhi::BufferHandle m_VertexBuffer;
	lm::rhi::BufferHandle m_IndexBuffer;

	lm::rhi::TextureHandle m_Texture;
	lm::rhi::SamplerHandle m_TextureSampler;

	std::vector<lm::rhi::BufferHandle> m_UniformBuffers;

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

		VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
		lm::rhi::TextureHandle depthTexture;

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> presentCompleteSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
	} m_Context;
	uint32_t m_FrameIndex = 0;
	bool m_FramebufferResized = false;

	// --- ImGui ---
	VkDescriptorPool m_ImGuiDescriptorPool;
};
