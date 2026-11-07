#include "lumen_engine/gfx/vulkan/vk_utils.hpp"

#include <fstream>
#include <stdexcept>

namespace lm {

void TransitionImage(VkCommandBuffer cmd, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
		VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess,
		VkImageAspectFlags aspect) {
	VkImageMemoryBarrier2 barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = srcStage,
		.srcAccessMask = srcAccess,
		.dstStageMask = dstStage,
		.dstAccessMask = dstAccess,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = { aspect, 0, 1, 0, 1 },
	};
	VkDependencyInfo dep{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier,
	};
	vkCmdPipelineBarrier2(cmd, &dep);
}

std::vector<uint32_t> ReadSpirV(const std::filesystem::path &path) {
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file) {
		throw std::runtime_error{ "failed to open shader: " + path.string() };
	}
	const size_t size = static_cast<size_t>(file.tellg());
	if (size == 0 || size % sizeof(uint32_t) != 0) {
		throw std::runtime_error{ "Invalid SPIR-V file: " + path.string() };
	}
	std::vector<uint32_t> buffer(size / sizeof(uint32_t));
	file.seekg(0);
	file.read(reinterpret_cast<char *>(buffer.data()),
			static_cast<std::streamsize>(size));
	return buffer;
}

} // namespace lm
