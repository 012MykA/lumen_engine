#include "vulkan_renderer_layer.hpp"

#include <lumen_engine/app/entry_point.hpp>

#include <print>

class SandboxApp : public lm::Application {
public:
	explicit SandboxApp(const lm::ApplicationCommandLineArgs &args) : lm::Application(args) {
		constexpr std::string_view buildType =
#if defined(LUMEN_DYNAMIC_LINK)
				"Dynamic Library";
#elif defined(LUMEN_STATIC_LINK)
				"Static Library";
#else
				"Unknown";
#endif
		std::println("Built as: {}", buildType);

		m_MainWindow = CreateWindow({});
		m_MainWindow->PushLayer<VulkanRendererLayer>(m_MainWindow);
	}

private:
	lm::Window *m_MainWindow;
};

std::unique_ptr<lm::Application> lm::CreateApplication(const lm::ApplicationCommandLineArgs &args) {
	return std::make_unique<SandboxApp>(args);
}
