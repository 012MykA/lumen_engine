#include "lumen_engine/rhi/vulkan_backend/vulkan_rendering_device.hpp"
#include "vulkan_translate.hpp"

#include <stdexcept>

namespace lm::rhi {

TextureHandle lm::rhi::VulkanRenderingDevice::CreateTexture(const TextureDesc &desc, const void *data) {
	if (desc.mipLevels != 1) {
		throw std::runtime_error("CreateTexture: mip level generation is not implemented yet");
	}

	VulkanTexture tex;
	tex.extent = { desc.width, desc.height };
	tex.format = TranslateTextureFormat(desc.format);

	VkImageUsageFlags usage = TranslateTextureUsage(desc.usage);
}

void VulkanRenderingDevice::DestroyTexture(TextureHandle handle) {
}

} // namespace lm::rhi
