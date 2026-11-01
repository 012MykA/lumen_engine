#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

namespace lm {

class VulkanPipeline final {
public:
	VulkanPipeline(VkDevice device, VkPipelineLayout layout, VkPipeline pipeline);
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline &) = delete;
	VulkanPipeline &operator=(const VulkanPipeline &) = delete;
	VulkanPipeline(VulkanPipeline &&other) noexcept;
	VulkanPipeline &operator=(VulkanPipeline &&other) noexcept;

	VkPipelineLayout GetVkPipelineLayout() const { return m_Layout; }
	VkPipeline GetVkPipeline() const { return m_Pipeline; }

private:
	void Destroy();

private:
	VkDevice m_Device{ VK_NULL_HANDLE };
	VkPipelineLayout m_Layout{ VK_NULL_HANDLE };
	VkPipeline m_Pipeline{ VK_NULL_HANDLE };
};

} // namespace lm
