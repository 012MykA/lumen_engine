#include "lumen_engine/gfx/mesh.hpp"

namespace lm {

Mesh::Mesh(VulkanUploader &uploader, VmaAllocator allocator,
		std::span<const Vertex> vertices,
		std::span<const uint32_t> indices) :
		m_IndexCount{ static_cast<uint32_t>(indices.size()) } {
	m_VertexBuffer = uploader.UploadBuffer(vertices.data(), vertices.size_bytes(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, allocator);
	m_IndexBuffer = uploader.UploadBuffer(indices.data(), indices.size_bytes(),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, allocator);
}

void Mesh::Draw(VkCommandBuffer cmd) const {
	VkBuffer vb = m_VertexBuffer.GetVkBuffer();
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &offset);
	vkCmdBindIndexBuffer(cmd, m_IndexBuffer.GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmd, m_IndexCount, 1, 0, 0, 0);
}

} // namespace lm
