#include "lumen_engine/windowing/window_manager.hpp"

namespace lm {

Window *WindowManager::CreateWindow(const WindowDesc &desc, WindowEventCallbackFn onEvent) {
	std::unique_ptr<Window> window = Window::Create(desc);
	Window *windowPtr = window.get();

	window->SetEventCallback([windowPtr, cb = std::move(onEvent)](Event &e) { cb(windowPtr, e); });

	m_Windows.push_back(std::move(window));
	return windowPtr;
}

void WindowManager::MarkForDestruction(Window *window) {
	auto it = std::find(m_PendingDestroy.begin(), m_PendingDestroy.end(), window);
	if (it == m_PendingDestroy.end()) {
		m_PendingDestroy.push_back(window);
	}
}

void WindowManager::ProcessPendingDestructions() {
	if (m_PendingDestroy.empty()) {
		return;
	}
	for (Window *toDelete : m_PendingDestroy) {
		std::erase_if(m_Windows, [toDelete](const auto &w) { return w.get() == toDelete; });
	}
	m_PendingDestroy.clear();
}

} // namespace lm
