#include "lumen_engine/gfx/primitives.hpp"

namespace lm {

MeshData MakeCube() {
	struct Face {
		glm::vec3 n, u, v;
	};
	constexpr Face faces[] = {
		{ { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 } },
		{ { 0, 0, -1 }, { -1, 0, 0 }, { 0, 1, 0 } },
		{ { 1, 0, 0 }, { 0, 0, -1 }, { 0, 1, 0 } },
		{ { -1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 } },
		{ { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, -1 } },
		{ { 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 1 } },
	};

	MeshData data;
	for (const Face &f : faces) {
		const uint32_t base = static_cast<uint32_t>(data.vertices.size());
		const glm::vec3 c = f.n * 0.5f, u = f.u * 0.5f, v = f.v * 0.5f;
		data.vertices.push_back({ c - u - v, f.n, { 0.0f, 1.0f } });
		data.vertices.push_back({ c + u - v, f.n, { 1.0f, 1.0f } });
		data.vertices.push_back({ c + u + v, f.n, { 1.0f, 0.0f } });
		data.vertices.push_back({ c - u + v, f.n, { 0.0f, 0.0f } });
		data.indices.insert(data.indices.end(), { base, base + 1, base + 2, base + 2, base + 3, base });
	}
	return data;
}

} // namespace lm
