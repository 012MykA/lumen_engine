#include "lumen_engine/runtime/application.hpp"

#include "lumen_engine/core/events/event_dispatcher.hpp"
#include "lumen_engine/core/events/window_events.hpp"
#include "lumen_engine/windowing/window.hpp"
#include "lumen_engine/windowing/window_context.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <thread>

namespace lm {

Application::Application(const ApplicationCommandLineArgs &args) : m_Args(args) {
	assert(!s_Instance && "Application already exists!");
	s_Instance = this;
	if (!WindowContext::Init()) {
		throw std::runtime_error("failed to initialize WindowContext");
	}
}

Application::~Application() {
	WindowContext::Shutdown();
	s_Instance = nullptr;
}

void Application::Run() {
	auto lastTime = std::chrono::steady_clock::now();

	while (m_Running) {
		auto now = std::chrono::steady_clock::now();
		float ts = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;

		for (auto &window : m_WindowManager) {
			window->OnUpdate();
			window->UpdateLayers(ts);
			window->RenderLayers();
		}

		m_WindowManager.ProcessPendingDestructions();
		if (m_WindowManager.IsEmpty()) {
			Close();
		}
	}
}

void Application::Close() {
	m_Running = false;
}

Window *Application::CreateWindow(const WindowDesc &desc) {
	Window *window = m_WindowManager.CreateWindow(desc, [this](Window *sender, Event &e) {
		OnWindowEvent(sender, e);
	});
	return window;
}

void Application::OnEvent(Window *sender, Event &e) {
	EventDispatcher dp(e);
	dp.Dispatch<WindowCloseEvent>([this, sender](WindowCloseEvent &event) {
		return OnWindowClose(sender, event);
	});
	dp.Dispatch<WindowFocusEvent>([this, sender](WindowFocusEvent &event) {
		return OnWindowFocus(sender, event);
	});
}

void Application::OnWindowEvent(Window *sender, Event &e) {
	EventDispatcher dp(e);
	dp.Dispatch<WindowCloseEvent>([this, sender](auto &ev) { return OnWindowClose(sender, ev); });
	dp.Dispatch<WindowFocusEvent>([this, sender](auto &ev) { return OnWindowFocus(sender, ev); });
}

bool Application::OnWindowClose(Window *sender, WindowCloseEvent &e) {
	m_WindowManager.MarkForDestruction(sender);
	return true;
}

bool Application::OnWindowFocus(Window *sender, WindowFocusEvent &e) {
	return true;
}

} // namespace lm
