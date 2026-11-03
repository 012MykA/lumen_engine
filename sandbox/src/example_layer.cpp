#include "example_layer.hpp"

#include "platform.hpp"

ExampleLayer::ExampleLayer() :
		Layer{ "ExampleLayer" },
		m_Renderer{ lm::Application::Get().GetWindow() } {
	const auto &window = lm::Application::Get().GetWindow();

	if (IsSystemDarkTheme()) {
		SetWindowDarkMode(window.handle);
	}

	lm::WindowExtent extent = window.GetExtent();
}

void ExampleLayer::OnEvent(lm::Event &event) {
	lm::EventDispatcher dp{ event };
	dp.Dispatch<lm::WindowResizeEvent>([this](lm::WindowResizeEvent &e) { return OnWindowResize(e); });
	dp.Dispatch<lm::KeyPressedEvent>([this](lm::KeyPressedEvent &e) { return OnKeyPressed(e); });
}

void ExampleLayer::OnRender() {
	m_Renderer.OnRender();
}

bool ExampleLayer::OnWindowResize(lm::WindowResizeEvent &e) {
	m_Renderer.OnResize(e.GetExtent());
	return false;
}

bool ExampleLayer::OnKeyPressed(lm::KeyPressedEvent &e) {
	if (e.HasModifier(lm::Modifiers::Ctrl) && e.GetKey() == lm::Key::W) {
		lm::Application::Get().Close();
	}

	return false;
}
