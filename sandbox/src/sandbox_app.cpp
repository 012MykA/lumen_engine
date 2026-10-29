#include "example_layer.hpp"

#include <lumen_engine/runtime/entry_point.hpp>

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
