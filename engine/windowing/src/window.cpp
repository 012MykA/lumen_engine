#include "lumen_engine/windowing/window.hpp"

#include "platform/glfw/glfw_window_driver.hpp"

namespace lm {

std::unique_ptr<Window> Window::Create(const WindowDesc &desc) {
	return std::make_unique<GlfwWindowDriver>(desc);
}

void Window::UpdateLayers(float ts) {
	for (auto &layer : m_LayerStack) {
		layer->OnUpdate(ts);
	}
}

void Window::RenderLayers() {
	for (auto &layer : m_LayerStack) {
		layer->OnRender();
	}

	for (auto &layer : m_LayerStack) {
		layer->OnUIRender();
	}
}

void Window::OnEvent(Event &e) {
	for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
		if (e.handled) {
			break;
		}
		(*it)->OnEvent(e);
	}

	if (m_EventCallback) {
		m_EventCallback(e);
	}
}

} // namespace lm
