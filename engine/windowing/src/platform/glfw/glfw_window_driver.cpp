#include "glfw_window_driver.hpp"

#include "lumen_engine/events/keyboard_events.hpp"
#include "lumen_engine/events/mouse_events.hpp"
#include "lumen_engine/events/window_events.hpp"

#include "platform/glfw/glfw_context.hpp"

#if defined(LUMEN_PLATFORM_WINDOWS)
	#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(LUMEN_PLATFORM_LINUX)
	#define GLFW_EXPOSE_NATIVE_X11
	#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <format>
#include <stdexcept>

namespace lm {

GlfwWindowDriver::GlfwWindowDriver(const WindowDesc &desc) {
	if (!GlfwContext::Init()) {
		throw std::runtime_error{ "failed to initialize GLFW" };
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_FOCUSED, desc.focused ? GLFW_TRUE : GLFW_FALSE);

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

void GlfwWindowDriver::WaitForEvents() {
	glfwWaitEvents();
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

void GlfwWindowDriver::SetExtent(WindowExtent extent) {
	glfwSetWindowSize(m_Handle, static_cast<int>(extent.width), static_cast<int>(extent.height));
}

WindowExtent GlfwWindowDriver::GetExtent() const {
	int w, h;
	glfwGetFramebufferSize(m_Handle, &w, &h);
	return { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
}

void *GlfwWindowDriver::GetNativeHandle() const {
#if defined(LUMEN_PLATFORM_WINDOWS)
	return reinterpret_cast<void *>(glfwGetWin32Window(m_Handle));
#elif defined(LUMEN_PLATFORM_LINUX)
	int platform = glfwGetPlatform();
	if (platform == GLFW_PLATFORM_WAYLAND) {
		return reinterpret_cast<void *>(glfwGetWaylandWindow(m_Handle));
	} else if (platform == GLFW_PLATFORM_X11) {
		return reinterpret_cast<void *>(static_cast<uintptr_t>(glfwGetX11Window(m_Handle)));
	}
	return nullptr;
#else
	return nullptr;
#endif
}

void GlfwWindowDriver::InitCallbacks() {
	glfwSetWindowCloseCallback(m_Handle, [](GLFWwindow *window) {
		auto *self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		WindowCloseEvent e{};
		self->OnEvent(e);
	});

	glfwSetWindowSizeCallback(m_Handle, [](GLFWwindow *window, int width, int height) {
		auto *self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		WindowResizeEvent e{ WindowExtent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) } };
		self->OnEvent(e);
	});

	glfwSetWindowFocusCallback(m_Handle, [](GLFWwindow *window, int focused) {
		auto *self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		if (focused) {
			WindowFocusEvent e{};
			self->OnEvent(e);
		} else {
			WindowLostFocusEvent e{};
			self->OnEvent(e);
		}
	});

	glfwSetWindowPosCallback(m_Handle, [](GLFWwindow *window, int xpos, int ypos) {
		auto *self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		WindowMovedEvent e{ xpos, ypos };
		self->OnEvent(e);
	});

	glfwSetKeyCallback(m_Handle, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
		auto *self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		switch (action) {
			case GLFW_PRESS: {
				KeyPressedEvent e{ static_cast<KeyCode>(key), 0 };
				self->OnEvent(e);
				break;
			}
			case GLFW_RELEASE: {
				KeyReleasedEvent e{ static_cast<KeyCode>(key) };
				self->OnEvent(e);
				break;
			}
			case GLFW_REPEAT: {
				KeyPressedEvent e{ static_cast<KeyCode>(key), static_cast<uint32_t>(1) };
				self->OnEvent(e);
				break;
			}
		}
	});

	glfwSetCharCallback(m_Handle, [](GLFWwindow *window, unsigned int codepoint) {
		auto *self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));
		KeyTypedEvent e{ static_cast<KeyCode>(codepoint) };
		self->OnEvent(e);
	});

	glfwSetCursorPosCallback(m_Handle, [](GLFWwindow *window, double xpos, double ypos) {
		auto *self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));
		MouseMovedEvent e{ static_cast<float>(xpos), static_cast<float>(ypos) };
		self->OnEvent(e);
	});

	glfwSetScrollCallback(m_Handle, [](GLFWwindow *window, double xoffset, double yoffset) {
		auto *self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));
		MouseScrolledEvent e{ static_cast<float>(xoffset), static_cast<float>(yoffset) };
		self->OnEvent(e);
	});

	glfwSetMouseButtonCallback(m_Handle, [](GLFWwindow *window, int button, int action, int mods) {
		auto *self = static_cast<GlfwWindowDriver *>(glfwGetWindowUserPointer(window));

		switch (action) {
			case GLFW_PRESS: {
				MouseButtonPressedEvent e{ static_cast<MouseCode>(button) };
				self->OnEvent(e);
				break;
			}
			case GLFW_RELEASE: {
				MouseButtonReleasedEvent e{ static_cast<MouseCode>(button) };
				self->OnEvent(e);
				break;
			}
		}
	});
}

} //namespace lm
