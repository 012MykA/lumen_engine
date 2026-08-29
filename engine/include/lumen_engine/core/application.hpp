#pragma once

#include "lumen_engine/core/export.hpp"

#include <memory>
#include <string>
#include <vector>

namespace lm {

struct LUMEN_API ApplicationCommandLineArgs {
	ApplicationCommandLineArgs(int argc, char **argv) : args(argv, argv + argc) {}

	const std::string &operator[](size_t idx) const { return args[idx]; }

	std::vector<std::string> args;
};

class Window;
struct WindowDesc;

class Event;
class WindowCloseEvent;
class WindowFocusEvent;

class LUMEN_API Application {
public:
	explicit Application(const ApplicationCommandLineArgs &args);
	virtual ~Application();

	Application(const Application &) = delete;
	Application &operator=(const Application &) = delete;

	Application(Application &&) = delete;
	Application &operator=(Application &&) = delete;

	void Run();
	void Close();

	Window *CreateWindow(const WindowDesc &desc);

	void OnEvent(Window *sender, Event &e);

	static Application &Get() { return *s_Instance; }

private:
	bool OnWindowClose(Window *sender, WindowCloseEvent &e);
	bool OnWindowFocus(Window *sender, WindowFocusEvent &e);

private:
	ApplicationCommandLineArgs m_Args;
	bool m_Running{ true };

	std::vector<std::unique_ptr<Window>> m_Windows;
	std::vector<Window *> m_WindowsToDelete;

private:
	inline static Application *s_Instance{ nullptr };
};

std::unique_ptr<Application> CreateApplication(const ApplicationCommandLineArgs &args);

} // namespace lm
