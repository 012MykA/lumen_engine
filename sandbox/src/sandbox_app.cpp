#include "platform.hpp"
#include "vulkan_render_layer.hpp"

#include <lumen_engine/runtime/entry_point.hpp>

class SandboxApp final : public lm::Application {
public:
	explicit SandboxApp(const lm::ApplicationCmdArgs &args) : lm::Application(args) {
		GLFWwindow *window = lm::Application::Get().GetWindowHandle();

		if (IsSystemDarkTheme()) {
			SetWindowDarkMode(window);
		}
	}
};

std::unique_ptr<lm::Application> lm::CreateApplication(
		const lm::ApplicationCmdArgs &args) {
	return std::make_unique<SandboxApp>(args);
}
