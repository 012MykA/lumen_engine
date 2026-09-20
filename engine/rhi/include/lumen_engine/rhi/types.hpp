#pragma once

#include <concepts>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

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
using SamplerHandle = Handle<struct SamplerTag>;
using ShaderHandle = Handle<struct ShaderTag>;
using PipelineHandle = Handle<struct PipelineTag>;

enum class MemoryUsage : uint8_t {
	DeviceLocal,
	HostVisible,
};

template <typename T>
concept BitwiseEnum = requires(T a, T b) {
	{ a & b } -> std::same_as<T>;
};

template <BitwiseEnum T>
constexpr bool HasFlags(T mask, T flags) {
	return (mask & flags) == flags;
}

// --- Buffer ---
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

// --- Texture ---
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

// --- Sampler ---
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

// --- Shader ---
enum class ShaderStage : uint8_t {
	Vertex = 1 << 0,
	Fragment = 1 << 1,
	Compute = 1 << 2,
};
constexpr ShaderStage operator|(ShaderStage a, ShaderStage b) {
	using T = std::underlying_type_t<ShaderStage>;
	return static_cast<ShaderStage>(static_cast<T>(a) | static_cast<T>(b));
}
constexpr ShaderStage operator&(ShaderStage a, ShaderStage b) {
	using T = std::underlying_type_t<ShaderStage>;
	return static_cast<ShaderStage>(static_cast<T>(a) & static_cast<T>(b));
}

// --- Pipeline ---
struct PipelineShaderStageDesc {
	ShaderStage stage;
	ShaderHandle shaderModule;
	std::string entryPoint;
};

enum class VertexAttribFormat : uint8_t {
	Float,
	Float2,
	Float3,
	Float4,
};

struct VertexAttribute {
	uint32_t location;
	VertexAttribFormat format;
	uint32_t offset;
};

struct VertexBinding {
	uint32_t binding;
	uint32_t stride;
	std::vector<VertexAttribute> attributtes;
};

enum class PrimitiveTopology : uint8_t {
	TriangleList,
};

enum class CullMode : uint8_t {
	None,
	Front,
	Back,
};

enum class FrontFace : uint8_t {
	CounterClockwise,
	Clockwise,
};

enum class CompareOp {
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,
	Always
};

struct DepthStencilState {
	bool depthTestEnable = true;
	bool depthWriteEnable = true;
	CompareOp depthCompareOp = CompareOp::Less;
};

struct ColorBlendState {
	bool blendEnable = false;
};

} // namespace lm::rhi
