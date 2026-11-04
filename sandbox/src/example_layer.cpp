#include "example_layer.hpp"

#include "platform.hpp"

#define APP_WINDOW lm::Application::Get().GetWindow()

ExampleLayer::ExampleLayer() :
		Layer{ "ExampleLayer" },
		m_Renderer{ APP_WINDOW },
		m_Camera{ APP_WINDOW } {
	if (IsSystemDarkTheme()) {
		SetWindowDarkMode(APP_WINDOW.handle);
	}
}

void ExampleLayer::OnEvent(lm::Event &event) {
	m_Camera.OnEvent(event);

	lm::EventDispatcher dp{ event };
	dp.Dispatch<lm::WindowResizeEvent>([this](lm::WindowResizeEvent &e) { return OnWindowResize(e); });
	dp.Dispatch<lm::KeyPressedEvent>([this](lm::KeyPressedEvent &e) { return OnKeyPressed(e); });
}

void ExampleLayer::OnUpdate(float ts) {
	m_Camera.OnUpdate(ts);
	m_Renderer.SetViewProjection(m_Camera.GetViewProjection());
}

void ExampleLayer::OnRender() {
	m_Renderer.OnRender();
}

bool ExampleLayer::OnWindowResize(lm::WindowResizeEvent &e) {
	const auto extent = e.GetExtent();
	m_Renderer.OnResize(extent);
	if (extent.height > 0) {
		m_Camera.SetAspectRatio(static_cast<float>(extent.width) / extent.height);
	}
	return false;
}

bool ExampleLayer::OnKeyPressed(lm::KeyPressedEvent &e) {
	if (e.HasModifier(lm::Modifiers::Ctrl) && e.GetKey() == lm::Key::W) {
		lm::Application::Get().Close();
	}

	return false;
}
