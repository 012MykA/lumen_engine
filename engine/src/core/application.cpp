#include "lumen_engine/core/application.hpp"

#include "lumen_engine/core/window.hpp"
#include "lumen_engine/events/event_dispatcher.hpp"
#include "lumen_engine/events/window_events.hpp"
#include "lumen_engine/io/input.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <thread>

namespace lm {

Application::Application(const ApplicationCommandLineArgs &args) : m_Args(args) {
	assert(!s_Instance && "Application already exists!");
	s_Instance = this;
}

Application::~Application() {
	s_Instance = nullptr;
}

void Application::Run() {
	auto lastTime = std::chrono::steady_clock::now();

	while (m_Running) {
		auto now = std::chrono::steady_clock::now();
		float ts = std::chrono::duration<float>(now - lastTime).count();
		lastTime = now;

		for (auto &window : m_Windows) {
			window->OnUpdate();
			window->UpdateLayers(ts);
			window->RenderLayers();
		}

		if (!m_WindowsToDelete.empty()) {
			for (Window *toDelete : m_WindowsToDelete) {
				std::erase_if(m_Windows, [toDelete](const auto &w) {
					return w.get() == toDelete;
				});
			}
			m_WindowsToDelete.clear();

			if (m_Windows.empty()) {
				Close();
			}
		}
	}
}

void Application::Close() {
	m_Running = false;
}

Window *Application::CreateWindow(const WindowDesc &desc) {
	auto window = Window::Create(desc);
	Window *windowPtr = window.get();
	window->SetEventCallback([this, windowPtr](Event &e) {
		OnEvent(windowPtr, e);
	});

	m_Windows.push_back(std::move(window));

	Input::SetActiveWindow(windowPtr);

	return windowPtr;
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

bool Application::OnWindowClose(Window *sender, WindowCloseEvent &e) {
	auto it = std::find(m_WindowsToDelete.begin(), m_WindowsToDelete.end(), sender);
	if (it == m_WindowsToDelete.end()) {
		m_WindowsToDelete.push_back(sender);
	}

	return true;
}

bool Application::OnWindowFocus(Window *sender, WindowFocusEvent &e) {
	Input::SetActiveWindow(sender);
	return true;
}

} // namespace lm
