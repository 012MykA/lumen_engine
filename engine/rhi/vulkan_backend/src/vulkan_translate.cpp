#include "vulkan_translate.hpp"

#include <stdexcept>

namespace lm::rhi {

VkMemoryPropertyFlags TranslateMemoryUsage(MemoryUsage usage) {
	// clang-format off
	switch (usage) {
		case MemoryUsage::DeviceLocal:	return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		case MemoryUsage::HostVisible:	return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	// clang-format on
	throw std::runtime_error("TranslateMemoryUsage(): Unknown MemoryUsage");
}

VkBufferUsageFlags TranslateBufferUsage(BufferUsage usage) {
	VkBufferUsageFlags flags = 0;
	// clang-format off
	if (HasFlags(BufferUsage::VertexBuffer,  usage))	flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if (HasFlags(BufferUsage::IndexBuffer,   usage))	flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (HasFlags(BufferUsage::UniformBuffer, usage))	flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (HasFlags(BufferUsage::StorageBuffer, usage))	flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	// clang-format on
	return flags;
}

VkFormat TranslateFormat(Format format) {
	// clang-format off
	switch (format) {
		case Format::RGBA8_UNORM:			return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::RGBA8_SRGB:			return VK_FORMAT_R8G8B8A8_SRGB;
		case Format::BGRA8_UNORM:			return VK_FORMAT_B8G8R8A8_UNORM;
		case Format::BGRA8_SRGB:			return VK_FORMAT_B8G8R8A8_SRGB;
		case Format::RGBA16_SFLOAT:			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::RGBA32_SFLOAT:			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Format::D32_SFLOAT:			return VK_FORMAT_D32_SFLOAT;
		case Format::D24_UNORM_S8_UINT:		return VK_FORMAT_D24_UNORM_S8_UINT;
		case Format::D32_SFLOAT_S8_UINT:	return VK_FORMAT_D32_SFLOAT_S8_UINT;
		default:							return VK_FORMAT_UNDEFINED;
	}
	// clang-format on
}

VkImageUsageFlags TranslateTextureUsage(TextureUsage usage) {
	VkImageUsageFlags flags = 0;
	// clang-format off
	if (HasFlags(TextureUsage::Sampled, usage))					flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (HasFlags(TextureUsage::Storage, usage))					flags |= VK_IMAGE_USAGE_STORAGE_BIT;
	if (HasFlags(TextureUsage::ColorAttachment, usage))			flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (HasFlags(TextureUsage::DepthStencilAttachment, usage))	flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (HasFlags(TextureUsage::TransferSrc, usage))				flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (HasFlags(TextureUsage::TransferDst, usage))				flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	// clang-format on
	return flags;
}

VkFilter TranslateFilter(Filter filter) {
	// clang-format off
	switch (filter) {
		case Filter::Linear:	return VK_FILTER_LINEAR;
		case Filter::Nearest:	return VK_FILTER_NEAREST;
	}
	// clang-format on
	throw std::runtime_error("TranslateFilter(): Unknown Filter");
}

VkSamplerAddressMode TranslateAddressMode(AddressMode mode) {
	// clang-format off
	switch (mode) {
		case AddressMode::Repeat:				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case AddressMode::MirroredRepeat:		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case AddressMode::ClampToEdge:			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case AddressMode::ClampToBorder:		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	}
	// clang-format on
	throw std::runtime_error("TranslateAddressMode(): Unknown AddressMode");
}

VkShaderStageFlags TranslateShaderStageFlags(ShaderStage stage) {
	VkShaderStageFlags flags = 0;
	// clang-format off
	if (HasFlags(ShaderStage::Vertex,  stage))		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	if (HasFlags(ShaderStage::Fragment, stage))		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if (HasFlags(ShaderStage::Compute, stage))		flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	// clang-format on
	return flags;
}

VkShaderStageFlagBits TranslateShaderStageBits(ShaderStage stage) {
	// clang-format off
	switch (stage) {
		case ShaderStage::Vertex:		return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::Fragment:		return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::Compute:		return VK_SHADER_STAGE_COMPUTE_BIT;
	}
	// clang-format on
	throw std::runtime_error("TranslateShaderStageBits(): Unknown ShaderStage");
}

VkFormat TranslateVertexAttribFormat(VertexAttribFormat format) {
	// clang-format off
	switch (format) {
		case VertexAttribFormat::Float:		return VK_FORMAT_R32_SFLOAT;
		case VertexAttribFormat::Float2:	return VK_FORMAT_R32G32_SFLOAT;
		case VertexAttribFormat::Float3:	return VK_FORMAT_R32G32B32_SFLOAT;
		case VertexAttribFormat::Float4:	return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	// clang-format on
	throw std::runtime_error("TranslateVertexAttribFormat(): Unknown VertexAttribFormat");
}

VkPrimitiveTopology TranslatePrimitiveTopology(PrimitiveTopology topology) {
	// clang-format off
	switch (topology) {
		case PrimitiveTopology::TriangleList:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
	// clang-format on
	throw std::runtime_error("TranslatePrimitiveTopology(): Unknown PrimitiveTopology");
}

VkCullModeFlags TranslateCullMode(CullMode mode) {
	// clang-format off
	switch (mode) {
		case CullMode::None:	return VK_CULL_MODE_NONE;
		case CullMode::Front:	return VK_CULL_MODE_FRONT_BIT;
		case CullMode::Back:	return VK_CULL_MODE_BACK_BIT;
	}
	// clang-format on
	throw std::runtime_error("TranslateCullMode(): Unknown CullMode");
}

VkFrontFace TranslateFrontFace(FrontFace face) {
	// clang-format off
	switch (face) {
		case FrontFace::CounterClockwise:	return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		case FrontFace::Clockwise:			return VK_FRONT_FACE_CLOCKWISE;
	}
	// clang-format on
	throw std::runtime_error("TranslateFrontFace(): Unknown FrontFace");
}

VkCompareOp TranslateCompareOp(CompareOp op) {
	// clang-format off
	switch (op) {
		case CompareOp::Never:				return VK_COMPARE_OP_NEVER;
		case CompareOp::Less:				return VK_COMPARE_OP_LESS;
		case CompareOp::Equal:				return VK_COMPARE_OP_EQUAL;
		case CompareOp::LessOrEqual:		return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CompareOp::Greater:			return VK_COMPARE_OP_GREATER;
		case CompareOp::NotEqual:			return VK_COMPARE_OP_NOT_EQUAL;
		case CompareOp::GreaterOrEqual:		return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case CompareOp::Always:				return VK_COMPARE_OP_ALWAYS;
	}
	// clang-format on
	throw std::runtime_error("TranslateCompareOp(): Unknown CompareOp");
}

} // namespace lm::rhi
