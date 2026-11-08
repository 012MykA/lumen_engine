#include "lumen_engine/gfx/vulkan/vk_pipeline_cache.hpp"

#include "lumen_engine/core/log.hpp"

#include <fstream>
#include <vector>

namespace lm {

VulkanPipelineCache::VulkanPipelineCache(VkDevice device, const std::filesystem::path &file) :
		m_Device{ device },
		m_File{ file } {
	std::vector<char> data;
	if (std::ifstream in{ m_File, std::ios::binary | std::ios::ate }) {
		data.resize(static_cast<size_t>(in.tellg()));
		in.seekg(0);
		in.read(data.data(), static_cast<std::streamsize>(data.size()));
	}

	VkPipelineCacheCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.initialDataSize = data.size(),
		.pInitialData = data.empty() ? nullptr : data.data(),
	};
	CHECK_VK(vkCreatePipelineCache(m_Device, &createInfo, nullptr, &m_Cache));
	if (!data.empty()) {
		LUMEN_CORE_TRACE("Pipeline cache loaded: {}", file.string());
	}
}

VulkanPipelineCache::~VulkanPipelineCache() {
	try {
		Save();
	} catch (const std::exception &e) {
		LUMEN_CORE_ERROR("Failed to save pipeline cache: {}", e.what());
	}
	vkDestroyPipelineCache(m_Device, m_Cache, nullptr);
}

void VulkanPipelineCache::Save() const {
	size_t size = 0;
	CHECK_VK(vkGetPipelineCacheData(m_Device, m_Cache, &size, nullptr));
	std::vector<char> data(size);
	CHECK_VK(vkGetPipelineCacheData(m_Device, m_Cache, &size, data.data()));

	auto parent = m_File.parent_path();
	if (!parent.empty() && !std::filesystem::exists(parent)) {
		std::filesystem::create_directories(parent);
	}

	auto tmpFile = m_File;
	tmpFile.replace_extension(".tmp");
	bool writeSuccess = false;
	{
		std::ofstream out{ tmpFile, std::ios::binary | std::ios::trunc };
		if (out.is_open()) {
			out.write(data.data(), static_cast<std::streamsize>(data.size()));
			writeSuccess = out.good();
		}
	}

	if (writeSuccess) {
		std::error_code ec;
		std::filesystem::rename(tmpFile, m_File, ec);
		if (ec) {
			LUMEN_CORE_ERROR("failed to replace pipeline cache: {}", ec.message());
			std::filesystem::remove(tmpFile, ec);
		}
	} else {
		std::error_code ec;
		std::filesystem::remove(tmpFile, ec);
	}
}

} // namespace lm
