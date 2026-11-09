#pragma once

#include "lumen_engine/gfx/vulkan/vk_buffer.hpp"
#include "lumen_engine/gfx/vulkan/vk_common.hpp"
#include "lumen_engine/gfx/vulkan/vk_uploader.hpp"
#include "lumen_engine/gfx/vulkan/vk_vertex.hpp"

#include <cstdint>
#include <span>

namespace lm {

class Mesh final {
public:
	Mesh() = default;
	Mesh(VulkanUploader &uploader, VmaAllocator allocator,
			std::span<const Vertex> vertices,
			std::span<const uint32_t> indices);

	void Draw(VkCommandBuffer cmd) const;

private:
	VulkanBuffer m_VertexBuffer, m_IndexBuffer;
	uint32_t m_IndexCount{ 0 };
};

} // namespace lm
