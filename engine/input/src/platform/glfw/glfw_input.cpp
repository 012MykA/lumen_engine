#include "lumen_engine/input/input.hpp"
#include "lumen_engine/windowing/window.hpp"

#include <GLFW/glfw3.h>

namespace lm {

// TODO: Rework input system

bool Input::IsKeyPressed(KeyCode key) {
	if (!s_ActiveWindow) {
		return false;
	}
	auto *handle = static_cast<GLFWwindow *>(s_ActiveWindow->GetGLFWwindow());
	int state = glfwGetKey(handle, static_cast<int>(key));
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool lm::Input::IsMouseButtonPressed(MouseCode button) {
	if (!s_ActiveWindow) {
		return false;
	}
	auto *handle = static_cast<GLFWwindow *>(s_ActiveWindow->GetGLFWwindow());
	int state = glfwGetMouseButton(handle, static_cast<int>(button));
	return state == GLFW_PRESS;
}

std::pair<float, float> Input::GetMousePosition() {
	if (!s_ActiveWindow) {
		return { 0.0f, 0.0f };
	}
	auto *handle = static_cast<GLFWwindow *>(s_ActiveWindow->GetGLFWwindow());
	double x, y;
	glfwGetCursorPos(handle, &x, &y);
	return { static_cast<float>(x), static_cast<float>(y) };
}

float Input::GetMouseX() {
	return GetMousePosition().first;
}

float Input::GetMouseY() {
	return GetMousePosition().second;
}

} // namespace lm
