#include "lumen_engine/gfx/vulkan/vk_pipeline.hpp"

#include <utility> // for std::exchange

namespace lm {

VulkanPipeline::VulkanPipeline(VkDevice device, VkPipelineLayout layout, VkPipeline pipeline) :
		m_Device{ device },
		m_Layout{ layout },
		m_Pipeline{ pipeline } {}

VulkanPipeline::~VulkanPipeline() {
	Destroy();
}

VulkanPipeline::VulkanPipeline(VulkanPipeline &&other) noexcept :
		m_Device{ std::exchange(other.m_Device, VK_NULL_HANDLE) },
		m_Layout{ std::exchange(other.m_Layout, VK_NULL_HANDLE) },
		m_Pipeline{ std::exchange(other.m_Pipeline, VK_NULL_HANDLE) } {}

VulkanPipeline &VulkanPipeline::operator=(VulkanPipeline &&other) noexcept {
	if (this != &other) {
		Destroy();
		m_Device = std::exchange(other.m_Device, VK_NULL_HANDLE);
		m_Layout = std::exchange(other.m_Layout, VK_NULL_HANDLE);
		m_Pipeline = std::exchange(other.m_Pipeline, VK_NULL_HANDLE);
	}
	return *this;
}

void VulkanPipeline::Destroy() {
	if (m_Device == VK_NULL_HANDLE) {
		return;
	}
	vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
	m_Device = VK_NULL_HANDLE;
	m_Pipeline = VK_NULL_HANDLE;
	m_Layout = VK_NULL_HANDLE;
}

} // namespace lm
