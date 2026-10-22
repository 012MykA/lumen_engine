#include "lumen_engine/core/window.hpp"

#include <stdexcept>

namespace lm {

void Window::Create(const WindowDesc &desc) {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	handle = glfwCreateWindow(desc.width, desc.height, desc.title.c_str(), nullptr, nullptr);
	if (!handle) {
		throw std::runtime_error{ "failed to create window" };
	}
}

void Window::Destroy() {
	glfwDestroyWindow(handle);
}

WindowExtent Window::GetExtent() const {
	int w, h;
	glfwGetFramebufferSize(handle, &w, &h);
	return {
		static_cast<uint32_t>(w), static_cast<uint32_t>(h)
	};
}

} // namespace lm
