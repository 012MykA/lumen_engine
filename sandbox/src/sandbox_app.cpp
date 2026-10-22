#include "platform.hpp"
#include "vulkan_render_layer.hpp"

#include <lumen_engine/runtime/entry_point.hpp>

class SandboxApp final : public lm::Application {
public:
	explicit SandboxApp(const lm::ApplicationCmdArgs &args) : lm::Application(args) {
		auto &app = lm::Application::Get();

		const auto &window = app.GetWindow();

		app.PushLayer<VulkanRenderLayer>(window);

		if (IsSystemDarkTheme()) {
			SetWindowDarkMode(window.handle);
		}
	}
};

std::unique_ptr<lm::Application> lm::CreateApplication(
		const lm::ApplicationCmdArgs &args) {
	return std::make_unique<SandboxApp>(args);
}
