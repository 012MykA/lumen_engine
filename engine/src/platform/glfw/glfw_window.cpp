#include "glfw_window.hpp"

#include "lumen_engine/events/window_events.hpp"

#include "platform/glfw/glfw_context.hpp"

#include <stdexcept>

namespace lm {

GlfwWindowDriver::GlfwWindowDriver(const WindowDesc &desc) {
	if (!GlfwContext::Init()) {
		throw std::runtime_error{ "failed to initialize GLFW" };
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);

	m_Handle = glfwCreateWindow(
			static_cast<int>(desc.extent.width),
			static_cast<int>(desc.extent.height),
			desc.title.c_str(), nullptr, nullptr);
	if (!m_Handle) {
		GlfwContext::Shutdown();
		throw std::runtime_error{ "failed to create window" };
	}

	glfwSetWindowUserPointer(m_Handle, this);
	InitCallbacks();
}

GlfwWindowDriver::~GlfwWindowDriver() {
	if (m_Handle) {
		glfwDestroyWindow(m_Handle);
	}
	GlfwContext::Shutdown();
}

void GlfwWindowDriver::OnUpdate() {
	glfwPollEvents();
}

bool GlfwWindowDriver::ShouldClose() const {
	return glfwWindowShouldClose(m_Handle);
}

void GlfwWindowDriver::SetTitle(std::string_view title) {
	glfwSetWindowTitle(m_Handle, std::string{ title }.c_str());
}

std::string_view GlfwWindowDriver::GetTitle() const {
	return glfwGetWindowTitle(m_Handle);
}

void GlfwWindowDriver::SetSize(WindowExtent extent) {
	glfwSetWindowSize(m_Handle, static_cast<int>(extent.width), static_cast<int>(extent.height));
}

WindowExtent GlfwWindowDriver::GetExtent() const {
	int w, h;
	glfwGetFramebufferSize(m_Handle, &w, &h);
	return { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
}

void GlfwWindowDriver::InitCallbacks() {
	glfwSetWindowCloseCallback(m_Handle, [](GLFWwindow *window) {
		auto self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		WindowCloseEvent e{};
		self->OnEvent(e);
	});

	glfwSetWindowSizeCallback(m_Handle, [](GLFWwindow *window, int width, int height) {
		auto self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		WindowResizeEvent e{ WindowExtent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) } };
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
