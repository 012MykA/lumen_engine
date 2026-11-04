#pragma once

#include "lumen_engine/core/input/key_codes.hpp"
#include "lumen_engine/core/input/mouse_codes.hpp"

#include <glm/glm.hpp>

#include <utility>

namespace lm {

class Input final {
public:
	static bool IsKeyPressed(KeyCode key);

	static bool IsMouseButtonPressed(MouseCode button);
	static std::pair<float, float> GetMousePosition();

	static void SetCursorCaptured(bool captured);
	static bool IsCursorCaptured();
};

} // namespace lm
