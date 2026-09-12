#pragma once

#include <cstdint>
#include <limits>
#include <string_view>

namespace lm::rhi {

enum class GraphicsAPI : uint8_t {
	Vulkan,
	D3D11,
	D3D12,
	Metal,
	OpenGL,
};

[[nodiscard]] constexpr std::string_view ToString(GraphicsAPI api) noexcept {
	// clang-format off
	switch (api) {
		case GraphicsAPI::Vulkan:	return "Vulkan";
		case GraphicsAPI::D3D11:	return "D3D11";
		case GraphicsAPI::D3D12:	return "D3D12";
		case GraphicsAPI::Metal:	return "Metal";
		case GraphicsAPI::OpenGL:	return "OpenGL";
		default:					return "Unknown";
	}
	// clang-format on
}

} // namespace lm::rhi
