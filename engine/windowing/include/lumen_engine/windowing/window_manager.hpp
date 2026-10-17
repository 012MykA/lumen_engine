#pragma once

#include "lumen_engine/windowing/window.hpp"
#include "lumen_engine/windowing/window_context.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace lm {

class WindowManager final {
public:
	using WindowEventCallbackFn = std::function<void(Window *, Event &)>;

	WindowManager() = default;
	WindowManager(const WindowManager &) = delete;
	WindowManager &operator=(const WindowManager &) = delete;
	WindowManager(WindowManager &&) = delete;
	WindowManager &operator=(WindowManager &&) = delete;

	Window *CreateWindow(const WindowDesc &desc, WindowEventCallbackFn onEvent);
	void MarkForDestruction(Window *window);
	void ProcessPendingDestructions();

	bool IsEmpty() const { return m_Windows.empty(); }

	auto begin() { return m_Windows.begin(); }
	auto end() { return m_Windows.end(); }

private:
	WindowContext m_Context;
	std::vector<std::unique_ptr<Window>> m_Windows;
	std::vector<Window *> m_PendingDestroy;
};

} // namespace lm
