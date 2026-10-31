#include "lumen_engine/core/window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cassert>
#include <stdexcept>

namespace lm {

Window::~Window() {
	Destroy();
}

void Window::Create(const WindowDesc &desc) {
	assert(handle == nullptr);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	handle = glfwCreateWindow(desc.width, desc.height, desc.title.c_str(), nullptr, nullptr);
	if (!handle) {
		throw std::runtime_error{ "failed to create window" };
	}
}

void Window::Destroy() {
	if (!handle) {
		return;
	}
	glfwDestroyWindow(handle);
	handle = nullptr;
}

WindowExtent Window::GetExtent() const {
	assert(handle != nullptr);
	int w, h;
	glfwGetFramebufferSize(handle, &w, &h);
	return {
		static_cast<uint32_t>(w), static_cast<uint32_t>(h)
	};
}

// --------------------
// --- GLFW Context ---
// --------------------

namespace {

void glfw_error_callback(int error_code, const char *description) {
	LUMEN_CORE_ERROR("GLFW error:\n  core: {}\n  description: {}", error_code, description);
}

} //namespace

GlfwContext::GlfwContext() {
	assert(s_Instance == nullptr && "GlfwContext already exists!");

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		throw std::runtime_error{ "failed to initialize GLFW" };
	}

	if (!glfwVulkanSupported()) {
		LUMEN_CORE_ERROR("GLFW: Vulkan not supported");
	}

	s_Instance = this;
}

GlfwContext::~GlfwContext() {
	glfwTerminate();
	s_Instance = nullptr;
}

GlfwContext &GlfwContext::Get() {
	assert(s_Instance != nullptr && "GlfwContext is not initialized!");
	return *s_Instance;
}

} // namespace lm
