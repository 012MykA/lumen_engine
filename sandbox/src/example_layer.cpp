#include "example_layer.hpp"

#include "platform.hpp"

ExampleLayer::ExampleLayer() : Layer{ "ExampleLayer" } {
	const auto &window = lm::Application::Get().GetWindow();

	if (IsSystemDarkTheme()) {
		SetWindowDarkMode(window.handle);
	}

	lm::WindowExtent extent = window.GetExtent();
	LUMEN_INFO("Window extent: {}x{}", extent.width, extent.height);
}

void ExampleLayer::OnEvent(lm::Event &event) {
	lm::EventDispatcher dp{ event };
	dp.Dispatch<lm::WindowResizeEvent>([](lm::WindowResizeEvent &e) {
		LUMEN_INFO("Resize: {}x{}", e.GetWidth(), e.GetHeight());
		return false;
	});
	dp.Dispatch<lm::KeyPressedEvent>([](lm::KeyPressedEvent &e) {
		LUMEN_INFO("{}", e.ToString());
		return false;
	});
	dp.Dispatch<lm::KeyTypedEvent>([](lm::KeyTypedEvent &e) {
		LUMEN_INFO("{}", e.ToString());
		return false;
	});
}
