#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/io/key_codes.hpp"
#include "lumen_engine/io/mouse_codes.hpp"

#include <utility> // for std::pair

namespace lm {

class Window;

class LUMEN_API Input {
public:
	Input(const Input &) = delete;
	Input &operator=(const Input &) = delete;

	static bool IsKeyPressed(KeyCode key);
	static bool IsMouseButtonPressed(MouseCode button);

	static std::pair<float, float> GetMousePosition();
	static float GetMouseX();
	static float GetMouseY();

	static void SetActiveWindow(Window *window) { s_ActiveWindow = window; }

private:
	Input() = default;
	static inline Window *s_ActiveWindow{ nullptr };
};

} // namespace lm
