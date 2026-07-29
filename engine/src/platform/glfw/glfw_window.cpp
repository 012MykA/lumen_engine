#include "glfw_window.hpp"

#include "lumen_engine/events/window_events.hpp"

#include "platform/glfw/glfw_context.hpp"

#include <stdexcept>

namespace lm {

GlfwWindowDriver::GlfwWindowDriver(const WindowDesc &desc) : m_Width(desc.width), m_Height(desc.height) {
	if (!GlfwContext::Init()) {
		throw std::runtime_error("failed to initialize GLFW");
	}

	m_Handle = glfwCreateWindow(
			static_cast<int>(m_Width),
			static_cast<int>(m_Height),
			desc.title.c_str(),
			nullptr, nullptr);
	if (!m_Handle) {
		throw std::runtime_error("failed to create window");
	}

	glfwSetWindowUserPointer(m_Handle, this);
	InitCallbacks();
}

GlfwWindowDriver::~GlfwWindowDriver() {
	if (m_Handle) {
		glfwDestroyWindow(m_Handle);
	}
}

void GlfwWindowDriver::OnUpdate() {
	glfwPollEvents();
}

bool GlfwWindowDriver::ShouldClose() const {
	return glfwWindowShouldClose(m_Handle);
}

void GlfwWindowDriver::InitCallbacks() {
	glfwSetWindowCloseCallback(m_Handle, [](GLFWwindow *window) {
		auto self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		WindowCloseEvent e{};
		self->OnEvent(e);
	});

	glfwSetWindowSizeCallback(m_Handle, [](GLFWwindow *window, int width, int height) {
		auto self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));
		self->m_Width = static_cast<uint32_t>(width);
		self->m_Height = static_cast<uint32_t>(height);

		WindowResizeEvent e{ self->m_Width, self->m_Height };
		self->OnEvent(e);
	});

	glfwSetWindowFocusCallback(m_Handle, [](GLFWwindow *window, int focused) {
		auto self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		if (focused) {
			WindowFocusEvent e{};
			self->OnEvent(e);
		} else {
			WindowLostFocusEvent e{};
			self->OnEvent(e);
		}
	});

	glfwSetWindowPosCallback(m_Handle, [](GLFWwindow *window, int xpos, int ypos) {
		auto self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		WindowMovedEvent e{ xpos, ypos };
		self->OnEvent(e);
	});
}

} //namespace lm
