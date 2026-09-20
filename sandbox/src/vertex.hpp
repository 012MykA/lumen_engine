#pragma once

#include "lumen_engine/lumen_engine.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <array>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;

	static constexpr lm::rhi::VertexBinding GetVertexBinding() {
		return {
			.binding = 0,
			.stride = sizeof(Vertex),
			.attributtes = {
					{ 0, lm::rhi::VertexAttribFormat::Float3, offsetof(Vertex, position) },
					{ 1, lm::rhi::VertexAttribFormat::Float3, offsetof(Vertex, normal) },
					{ 2, lm::rhi::VertexAttribFormat::Float2, offsetof(Vertex, uv) },
			},
		};
	}
};
