#pragma once

#include <glm/glm.hpp>

namespace lm {

struct PushConstants {
	glm::mat4 viewProj;
	glm::mat4 model;
};

static_assert(sizeof(PushConstants) <= 128);

} // namespace lm
