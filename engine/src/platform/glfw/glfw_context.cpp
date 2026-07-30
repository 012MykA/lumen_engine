#include "platform/glfw/glfw_context.hpp"

#include <GLFW/glfw3.h>

#include <print>

namespace lm {

bool GlfwContext::Init() {
	if (s_RefCount++ == 0) {
		glfwSetErrorCallback([](int code, const char *desc) {
			std::println(stderr, "GLFW Error ({}): {}", code, desc);
		});

		if (!glfwInit()) {
			s_RefCount--;
			return false;
		}
	}

	return true;
}

void GlfwContext::Shutdown() {
	if (s_RefCount > 0 && --s_RefCount == 0) {
		glfwTerminate();
	}
}

} // namespace lm
