#include "platform.hpp"

#include <lumen_engine/lumen_engine.hpp>
#include <lumen_engine/runtime/entry_point.hpp>

class ExampleLayer : public lm::Layer {
public:
	ExampleLayer() : Layer{ "ExampleLayer" } {
		const auto &window = lm::Application::Get().GetWindow();

		if (IsSystemDarkTheme()) {
			SetWindowDarkMode(window.handle);
		}

		lm::WindowExtent extent = window.GetExtent();
		LUMEN_INFO("Window extent: {}x{}", extent.width, extent.height);
	}

	void OnEvent(lm::Event &event) {
		lm::EventDispatcher dp{ event };
		dp.Dispatch<lm::WindowResizeEvent>([](lm::WindowResizeEvent &e) {
			LUMEN_INFO("Resize: {}x{}", e.GetWidth(), e.GetHeight());
			return false;
		});
	}
};

class SandboxApp final : public lm::Application {
public:
	explicit SandboxApp(const lm::ApplicationCmdArgs &args) : lm::Application(args) {
		auto &app = lm::Application::Get();
		app.PushLayer<ExampleLayer>();
	}
};

std::unique_ptr<lm::Application> lm::CreateApplication(
		const lm::ApplicationCmdArgs &args) {
	return std::make_unique<SandboxApp>(args);
}
