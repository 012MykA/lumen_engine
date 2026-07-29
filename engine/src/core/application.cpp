#include "lumen_engine/core/application.hpp"

#include "lumen_engine/events/event_dispatcher.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <print>
#include <thread>

namespace lm {

class TestLayer : public Layer {
public:
	TestLayer() : Layer("TestLayer") {}

	virtual void OnEvent(Event &e) override {
		std::println("{}", e.ToString());
	}
};

Application::Application(const ApplicationCommandLineArgs &args) : m_Args(args) {
	assert(!s_Instance && "Application already exists!");
	s_Instance = this;

	auto mainWin = CreateWindow(WindowDesc{});
	mainWin->GetLayerStack().PushLayer(new TestLayer{});

	auto secWin = CreateWindow(WindowDesc{});
}

Application::~Application() {
	s_Instance = nullptr;
}

void Application::Run() {
	while (m_Running) {
		for (auto &window : m_Windows) {
			window->OnUpdate();
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
	return windowPtr;
}

void Application::OnEvent(Window *sender, Event &e) {
	EventDispatcher dp(e);
	dp.Dispatch<WindowCloseEvent>([this, sender](WindowCloseEvent &event) {
		return OnWindowClose(sender, event);
	});
}

bool Application::OnWindowClose(Window *sender, WindowCloseEvent &e) {
	auto it = std::find(m_WindowsToDelete.begin(), m_WindowsToDelete.end(), sender);
	if (it == m_WindowsToDelete.end()) {
		m_WindowsToDelete.push_back(sender);
	}

	return true;
}

} // namespace lm
