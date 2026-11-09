#pragma once

#include "lumen_engine/gfx/vulkan/vk_vertex.hpp"

#include <cstdint>
#include <vector>

namespace lm {

struct MeshData {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

MeshData MakeCube();

} // namespace lm
