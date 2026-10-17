#include "lumen_engine/core/log.hpp"
#include "lumen_engine/windowing/window_context.hpp"

#include <GLFW/glfw3.h>

namespace lm {

WindowContext::WindowContext() {
	if (!Init()) {
		throw std::runtime_error("failed to initialize WindowContext");
	}
}

WindowContext::~WindowContext() {
	Shutdown();
}

bool WindowContext::Init() {
	if (s_RefCount++ == 0) {
		glfwSetErrorCallback([](int code, const char *desc) {
			LUMEN_CORE_ERROR("GLFW Error ({}): {}", code, desc);
		});

		if (!glfwInit()) {
			s_RefCount--;
			return false;
		}
	}

	return true;
}

void WindowContext::Shutdown() {
	if (s_RefCount > 0 && --s_RefCount == 0) {
		glfwTerminate();
	}
}

bool WindowContext::IsInitialized() {
	return s_RefCount > 0;
}

} // namespace lm
