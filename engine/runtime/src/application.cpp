#include "lumen_engine/runtime/application.hpp"

#include "lumen_engine/core/events/event.hpp"
#include "lumen_engine/core/events/event_dispatcher.hpp"
#include "lumen_engine/core/events/keyboard_events.hpp"
#include "lumen_engine/core/events/mouse_events.hpp"
#include "lumen_engine/core/events/window_events.hpp"

#include <cassert>
#include <chrono>
#include <stdexcept>

namespace lm {

Application::Application(const ApplicationCmdArgs &args) : m_Args{ args } {
	assert(!s_Instance && "Application already exists!");
	s_Instance = this;
	Init();
}

Application::~Application() {
	Shutdown();
	s_Instance = nullptr;
}

Application &Application::Get() {
	return *s_Instance;
}

void Application::Run() {
	m_Running = true;

	auto lastTime = std::chrono::steady_clock::now();

	while (m_Running && !glfwWindowShouldClose(m_Window.handle)) {
		glfwPollEvents();

		auto now = std::chrono::steady_clock::now();
		float ts = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;

		for (auto &layer : m_LayerStack) {
			layer->OnUpdate(ts);
		}

		for (auto &layer : m_LayerStack) {
			layer->OnRender();
		}

		for (auto &layer : m_LayerStack) {
			layer->OnUIRender();
		}
	}
}

void Application::Close() {
	m_Running = false;
}

void Application::Init() {
	if (!glfwInit()) {
		throw std::runtime_error{ "failed to initialize GLFW" };
	}

	m_Window.Create();

	glfwSetWindowUserPointer(m_Window.handle, this);
	InitGlfwCallbacks(m_Window.handle);
}

void Application::Shutdown() {
	m_Window.Destroy();
	glfwTerminate();
}

void Application::OnEvent(Event &e) {
	EventDispatcher dp{ e };
	dp.Dispatch<WindowCloseEvent>([this](WindowCloseEvent &) { Close(); return false; });

	for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
		if (e.handled) {
			break;
		}
		(*it)->OnEvent(e);
	}
}

void Application::InitGlfwCallbacks(GLFWwindow *handle) {
	glfwSetWindowCloseCallback(handle, [](GLFWwindow *window) {
		auto *self = static_cast<Application *>(glfwGetWindowUserPointer(window));

		WindowCloseEvent e{};
		self->OnEvent(e);
	});

	glfwSetWindowSizeCallback(handle, [](GLFWwindow *window, int width, int height) {
		auto *self = static_cast<Application *>(glfwGetWindowUserPointer(window));

		WindowResizeEvent e{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		self->OnEvent(e);
	});

	glfwSetWindowFocusCallback(handle, [](GLFWwindow *window, int focused) {
		auto *self = static_cast<Application *>(glfwGetWindowUserPointer(window));

		if (focused) {
			WindowFocusEvent e{};
			self->OnEvent(e);
		} else {
			WindowLostFocusEvent e{};
			self->OnEvent(e);
		}
	});

	glfwSetWindowPosCallback(handle, [](GLFWwindow *window, int xpos, int ypos) {
		auto *self = static_cast<Application *>(glfwGetWindowUserPointer(window));

		WindowMovedEvent e{ xpos, ypos };
		self->OnEvent(e);
	});

	glfwSetKeyCallback(handle, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
		auto *self = static_cast<Application *>(glfwGetWindowUserPointer(window));

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

	glfwSetCharCallback(handle, [](GLFWwindow *window, unsigned int codepoint) {
		auto *self = static_cast<Application *>(glfwGetWindowUserPointer(window));
		KeyTypedEvent e{ static_cast<KeyCode>(codepoint) };
		self->OnEvent(e);
	});

	glfwSetCursorPosCallback(handle, [](GLFWwindow *window, double xpos, double ypos) {
		auto *self = static_cast<Application *>(glfwGetWindowUserPointer(window));
		MouseMovedEvent e{ static_cast<float>(xpos), static_cast<float>(ypos) };
		self->OnEvent(e);
	});

	glfwSetScrollCallback(handle, [](GLFWwindow *window, double xoffset, double yoffset) {
		auto *self = static_cast<Application *>(glfwGetWindowUserPointer(window));
		MouseScrolledEvent e{ static_cast<float>(xoffset), static_cast<float>(yoffset) };
		self->OnEvent(e);
	});

	glfwSetMouseButtonCallback(handle, [](GLFWwindow *window, int button, int action, int mods) {
		auto *self = static_cast<Application *>(glfwGetWindowUserPointer(window));

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

} // namespace lm
