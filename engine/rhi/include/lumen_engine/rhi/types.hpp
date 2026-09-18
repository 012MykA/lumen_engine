#pragma once

#include <concepts>
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

template <typename Tag>
struct Handle {
	static constexpr uint32_t k_InvalidIndex = std::numeric_limits<uint32_t>::max();

	uint32_t index = k_InvalidIndex;
	uint32_t generation = 0;
};

using BufferHandle = Handle<struct BufferTag>;
using TextureHandle = Handle<struct TextureTag>;

enum class MemoryUsage : uint8_t {
	DeviceLocal,
	HostVisible,
};

template <typename T>
concept UsageFlags = requires(T a, T b) {
	{ a & b } -> std::same_as<T>;
};

template <UsageFlags T>
constexpr bool HasUsage(T flags, T usage) {
	return (flags & usage) == usage;
}

enum class BufferUsage : uint32_t {
	VertexBuffer = 1 << 0,
	IndexBuffer = 1 << 1,
	UniformBuffer = 1 << 2,
	StorageBuffer = 1 << 3,
};
constexpr BufferUsage operator|(BufferUsage a, BufferUsage b) {
	using T = std::underlying_type_t<BufferUsage>;
	return static_cast<BufferUsage>(static_cast<T>(a) | static_cast<T>(b));
}
constexpr BufferUsage operator&(BufferUsage a, BufferUsage b) {
	using T = std::underlying_type_t<BufferUsage>;
	return static_cast<BufferUsage>(static_cast<T>(a) & static_cast<T>(b));
}

enum class Format {
	Undefined,

	RGBA8_UNORM,
	RGBA8_SRGB,
	BGRA8_UNORM,
	BGRA8_SRGB,

	RGBA16_SFLOAT,
	RGBA32_SFLOAT,

	D32_SFLOAT,
	D24_UNORM_S8_UINT,
	D32_SFLOAT_S8_UINT,
};
constexpr uint32_t FormatBytesPerPixel(Format format) {
	// clang-format off
	switch (format) {
		case Format::RGBA8_UNORM:			
		case Format::RGBA8_SRGB:			
		case Format::BGRA8_UNORM:			
		case Format::BGRA8_SRGB:			return 4;
		case Format::RGBA16_SFLOAT:			return 8;
		case Format::RGBA32_SFLOAT:			return 16;
		default:							return 0;
	}
	// clang-format on
}

enum class TextureUsage {
	Sampled = 1 << 0,
	Storage = 1 << 1,
	ColorAttachment = 1 << 2,
	DepthStencilAttachment = 1 << 3,
	TransferSrc = 1 << 4,
	TransferDst = 1 << 5,
};
constexpr TextureUsage operator|(TextureUsage a, TextureUsage b) {
	using T = std::underlying_type_t<TextureUsage>;
	return static_cast<TextureUsage>(static_cast<T>(a) | static_cast<T>(b));
}
constexpr TextureUsage operator&(TextureUsage a, TextureUsage b) {
	using T = std::underlying_type_t<TextureUsage>;
	return static_cast<TextureUsage>(static_cast<T>(a) & static_cast<T>(b));
}

enum class Filter : uint8_t {
	Nearest,
	Linear,
};

enum class AddressMode : uint8_t {
	Repeat,
	MirroredRepeat,
	ClampToEdge,
	ClampToBorder,
};

} // namespace lm::rhi
