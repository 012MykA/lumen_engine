#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <filesystem>

namespace lm {

class VulkanPipelineCache final {
public:
	VulkanPipelineCache(VkDevice device, const std::filesystem::path &file);
	~VulkanPipelineCache();

	VulkanPipelineCache(const VulkanPipelineCache &) = delete;
	VulkanPipelineCache &operator=(const VulkanPipelineCache &) = delete;

	VkPipelineCache GetVkPipelineCache() const { return m_Cache; }

private:
	void Save() const;

private:
	VkDevice m_Device{ VK_NULL_HANDLE };
	VkPipelineCache m_Cache{ VK_NULL_HANDLE };
	std::filesystem::path m_File;
};

} // namespace lm
