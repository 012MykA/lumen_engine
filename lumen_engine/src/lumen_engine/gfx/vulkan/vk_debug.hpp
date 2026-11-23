#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <cstdint>
#include <string_view>

namespace lm::debug {

static_assert(sizeof(void *) == 8, "debug names require 64-bit handles");

struct Color {
	float r, g, b, a;
};
namespace color {

constexpr Color Pass{ 0.30f, 0.69f, 0.31f, 1.0f };
constexpr Color Transfer{ 0.25f, 0.55f, 0.90f, 1.0f };
constexpr Color Barrier{ 0.60f, 0.60f, 0.60f, 1.0f };
constexpr Color Ui{ 0.95f, 0.60f, 0.15f, 1.0f };

} // namespace color

void SetEnabled(bool enabled);
bool IsEnabled();

// --- Object names ---

template <typename T>
struct ObjectType;

#define LUMEN_DEBUG_OBJECT_TYPE(VkT, Enum) \
	template <> \
	struct ObjectType<VkT> { \
		static constexpr VkObjectType value = Enum; \
	};

LUMEN_DEBUG_OBJECT_TYPE(VkQueue, VK_OBJECT_TYPE_QUEUE)
LUMEN_DEBUG_OBJECT_TYPE(VkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER)
LUMEN_DEBUG_OBJECT_TYPE(VkSemaphore, VK_OBJECT_TYPE_SEMAPHORE)
LUMEN_DEBUG_OBJECT_TYPE(VkFence, VK_OBJECT_TYPE_FENCE)
LUMEN_DEBUG_OBJECT_TYPE(VkBuffer, VK_OBJECT_TYPE_BUFFER)
LUMEN_DEBUG_OBJECT_TYPE(VkImage, VK_OBJECT_TYPE_IMAGE)
LUMEN_DEBUG_OBJECT_TYPE(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW)
LUMEN_DEBUG_OBJECT_TYPE(VkSampler, VK_OBJECT_TYPE_SAMPLER)
LUMEN_DEBUG_OBJECT_TYPE(VkShaderModule, VK_OBJECT_TYPE_SHADER_MODULE)
LUMEN_DEBUG_OBJECT_TYPE(VkPipeline, VK_OBJECT_TYPE_PIPELINE)
LUMEN_DEBUG_OBJECT_TYPE(VkPipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT)
LUMEN_DEBUG_OBJECT_TYPE(VkPipelineCache, VK_OBJECT_TYPE_PIPELINE_CACHE)
LUMEN_DEBUG_OBJECT_TYPE(VkDescriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET)
LUMEN_DEBUG_OBJECT_TYPE(VkDescriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
LUMEN_DEBUG_OBJECT_TYPE(VkDescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL)
LUMEN_DEBUG_OBJECT_TYPE(VkCommandPool, VK_OBJECT_TYPE_COMMAND_POOL)
LUMEN_DEBUG_OBJECT_TYPE(VkSwapchainKHR, VK_OBJECT_TYPE_SWAPCHAIN_KHR)

#undef LUMEN_DEBUG_OBJECT_TYPE

void SetObjectName(VkDevice device, VkObjectType type, uint64_t handle, std::string_view name);

template <typename T>
void SetObjectName(VkDevice device, T handle, std::string_view name) {
	SetObjectName(device, ObjectType<T>::value, reinterpret_cast<uint64_t>(handle), name);
}

// --- Command Buffer Label ---

void BeginLabel(VkCommandBuffer cmd, const char *name, const Color &color);
void EndLabel(VkCommandBuffer cmd);

class ScopedLabel final {
public:
	ScopedLabel(VkCommandBuffer cmd, const char *name, const Color &color = color::Pass);
	~ScopedLabel();

	ScopedLabel(const ScopedLabel &) = delete;
	ScopedLabel &operator=(const ScopedLabel &) = delete;

private:
	VkCommandBuffer m_Cmd{ VK_NULL_HANDLE };
};

} // namespace lm::debug
