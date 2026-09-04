#include "vulkan_render_layer.hpp"

#include <lumen_engine/app/entry_point.hpp>

#ifdef CreateWindow
	#undef CreateWindow
#endif

static void SetWindowDarkMode(lm::Window *window);

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
		LUMEN_TRACE("Built as: {}", buildType);

		m_MainWindow = CreateWindow({});
		SetWindowDarkMode(m_MainWindow);

		m_MainWindow->PushLayer<VulkanRenderLayer>(m_MainWindow);
	}

private:
	lm::Window *m_MainWindow;
};

std::unique_ptr<lm::Application> lm::CreateApplication(const lm::ApplicationCommandLineArgs &args) {
	return std::make_unique<SandboxApp>(args);
}

#if defined(LUMEN_PLATFORM_WINDOWS)
	#include <Windows.h>

	#include <dwmapi.h>
	#pragma comment(lib, "dwmapi.lib")

void SetWindowDarkMode(lm::Window *window) {
	HWND hwnd = static_cast<HWND>(window->GetNativeHandle());
	BOOL useDarkMode = TRUE;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
}
#else
void SetWindowDarkMode(lm::Window *window, char value) {}
#endif
