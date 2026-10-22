#include "lumen_engine/runtime/application.hpp"

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

	while (m_Running) {
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

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_WindowHandle = glfwCreateWindow(1280, 720, "Lumen Engine", nullptr, nullptr);
	if (!m_WindowHandle) {
		throw std::runtime_error{ "failed to create window" };
	}

	// glfwSetWindowUserPointer(m_Handle, this);
	// InitCallbacks();
}

void Application::Shutdown() {
	glfwTerminate();
}

} // namespace lm
