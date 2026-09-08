#pragma once

#include <cstdint>

namespace lm::rhi {

enum class GraphicsAPI : uint8_t {
	Vulkan,
	D3D11,
	D3D12,
	Metal,
	OpenGL,
};

} // namespace lm::rhi
