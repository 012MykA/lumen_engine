#include "lumen_engine/rhi/device.hpp"

#include "lumen_engine/io/log.hpp"

#if defined(LUMEN_HAS_VULKAN)
	#include "lumen_engine/rhi/vulkan/vulkan_device.hpp"
#endif

#include <stdexcept>

namespace lm::rhi {

LUMEN_API std::unique_ptr<IDevice> CreateDevice(GraphicsAPI api, void *window) {
	switch (api) {
#if defined(LUMEN_HAS_VULKAN)
		case GraphicsAPI::Vulkan: {
			return std::make_unique<VulkanDevice>(window);
		}
#endif
		default:
			throw std::runtime_error("Unknown backend");
	}
}

LUMEN_API bool IsAvailableAPI(GraphicsAPI api) {
	switch (api) {
#if defined(LUMEN_HAS_VULKAN)
		case GraphicsAPI::Vulkan:
			return true;
#endif
		default:
			return false;
	}
}

} // namespace lm::rhi
