#include "example_layer.hpp"

#include "platform.hpp"

#define APP_WINDOW lm::Application::Get().GetWindow()

ExampleLayer::ExampleLayer() :
		Layer{ "ExampleLayer" },
		m_Renderer{ APP_WINDOW },
		m_Camera{ APP_WINDOW } {
	m_Camera.SetPosition({ 0.0f, 0.0f, 2.0f });

	if (IsSystemDarkTheme()) {
		SetWindowDarkMode(APP_WINDOW.handle);
	}
}

void ExampleLayer::OnEvent(lm::Event &event) {
	m_Camera.OnEvent(event);

	lm::EventDispatcher dp{ event };
	dp.Dispatch<lm::WindowResizeEvent>([this](lm::WindowResizeEvent &e) { return OnWindowResize(e); });
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
