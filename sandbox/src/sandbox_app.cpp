#include "platform.hpp"
#include "vulkan_render_layer.hpp"

#include <lumen_engine/runtime/entry_point.hpp>

class SandboxApp final : public lm::Application {
public:
	explicit SandboxApp(const lm::ApplicationCommandLineArgs &args) : lm::Application(args) {
		m_MainWindow = CreateWindow({});

		if (IsSystemDarkTheme()) {
			SetWindowDarkMode(m_MainWindow);
		}

		m_MainWindow->PushLayer<VulkanRenderLayer>();
	}

private:
	lm::Window *m_MainWindow;
};

std::unique_ptr<lm::Application> lm::CreateApplication(
		const lm::ApplicationCommandLineArgs &args) {
	return std::make_unique<SandboxApp>(args);
}
