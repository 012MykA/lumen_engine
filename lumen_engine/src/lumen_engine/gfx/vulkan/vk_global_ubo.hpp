#pragma once

#include <glm/glm.hpp>

namespace lm {

struct alignas(16) GlobalUBO { // std140
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewProj;
	glm::vec4 cameraPos; // xyz, w - unused
	glm::vec4 time; // x = total time, y = delta time
};
static_assert(alignof(GlobalUBO) == 16);

} // namespace lm
