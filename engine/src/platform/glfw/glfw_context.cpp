#include "platform/glfw/glfw_context.hpp"

#include <GLFW/glfw3.h>

namespace lm {

bool GlfwContext::Init() {
	if (s_Initialized) {
		return true;
	}

	if (!glfwInit()) {
		return false;
	}

	s_Initialized = true;
	return true;
}

void GlfwContext::Shutdown() {
	if (!s_Initialized) {
		return;
	}

	glfwTerminate();
	s_Initialized = false;
}

} // namespace lm
