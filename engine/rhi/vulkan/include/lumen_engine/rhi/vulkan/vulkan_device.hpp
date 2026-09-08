#pragma once

#include "lumen_engine/rhi/device.hpp"

namespace lm::rhi {

class VulkanDevice final : public IDevice {
public:
	GraphicsAPI GetAPI() const override { return GraphicsAPI::Vulkan; }
};

} // namespace lm::rhi
