#pragma once

#include "lumen_engine/core/input/key_codes.hpp"
#include "lumen_engine/core/input/mouse_codes.hpp"

#include <utility>

namespace lm {

class Input final {
public:
	static bool IsKeyDown(KeyCode key);
	static bool IsMouseButtonDown(MouseCode button);
	static std::pair<float, float> GetMousePosition();
	static void SetCursorCaptures(bool captured);
};

} // namespace lm
