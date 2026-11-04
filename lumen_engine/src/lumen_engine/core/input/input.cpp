#include "lumen_engine/core/input/input.hpp"

#include "lumen_engine/runtime/application.hpp"

#include <GLFW/glfw3.h>

namespace lm {

namespace {

GLFWwindow *Handle() {
	return Application::Get().GetWindow().handle;
}

} // namespace

bool Input::IsKeyPressed(KeyCode key) {
	return glfwGetKey(Handle(), key) == GLFW_PRESS;
}

bool Input::IsMouseButtonPressed(MouseCode button) {
	return glfwGetMouseButton(Handle(), button) == GLFW_PRESS;
}

std::pair<float, float> Input::GetMousePosition() {
	double x, y;
	glfwGetCursorPos(Handle(), &x, &y);
	return { static_cast<float>(x), static_cast<float>(y) };
}

void Input::SetCursorCaptured(bool captured) {
	glfwSetInputMode(Handle(), GLFW_CURSOR, captured ? GLFW_CURSOR_CAPTURED : GLFW_CURSOR_NORMAL);
}

bool Input::IsCursorCaptured() {
	return glfwGetInputMode(Handle(), GLFW_CURSOR) == GLFW_CURSOR_CAPTURED;
}

} // namespace lm
