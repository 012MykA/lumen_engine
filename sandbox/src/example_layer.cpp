#include "example_layer.hpp"

#include "platform.hpp"

#define APP_WINDOW lm::Application::Get().GetWindow()

ExampleLayer::ExampleLayer() :
		Layer{ "ExampleLayer" },
		m_Renderer{ APP_WINDOW },
		m_Camera{ APP_WINDOW },
		m_Cube{ m_Renderer.CreateMesh(lm::MakeCube()) } {
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
	m_Time += ts;

	m_Renderer.BeginScene(m_Camera.GetViewProjection());
	int rows = 10, cols = 10;
	for (int i = 0; i < cols; i++) {
		for (int j = 0; j < rows; j++) {
			glm::mat4 model = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ i * 2.0f, j * 2.0f, -4.0f });
			model = glm::rotate(model, m_Time * (0.5f + 0.25f * static_cast<float>(i)), glm::vec3{ 0.0f, 1.0f, 0.0f });
			m_Renderer.Submit(m_Cube, model);
		}
	}
}

void ExampleLayer::OnRender() {
	m_Renderer.OnRender();
}

bool ExampleLayer::OnWindowResize(lm::WindowResizeEvent &e) {
	m_Renderer.OnResize();
	return false;
}
