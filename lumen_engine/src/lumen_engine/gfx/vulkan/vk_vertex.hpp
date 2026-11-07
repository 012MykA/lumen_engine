#pragma once

#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <glm/glm.hpp>

#include <array>
#include <cstddef> // for offsetof

namespace lm {

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static constexpr VkVertexInputBindingDescription GetBindingDescription() {
		return { .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
	}

	static constexpr std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
		return {
			VkVertexInputAttributeDescription{ .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, pos) },
			VkVertexInputAttributeDescription{ .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color) },
			VkVertexInputAttributeDescription{ .location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, texCoord) },
		};
	}
};

} // namespace lm
