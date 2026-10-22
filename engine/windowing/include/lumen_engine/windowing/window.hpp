#pragma once

#include "lumen_engine/core/events/event.hpp"
#include "lumen_engine/core/layer_stack.hpp"
#include "lumen_engine/core/window_desc.hpp"

#include <functional>
#include <memory>
#include <string_view>

namespace lm {

class Window {
public:
	using EventCallbackFn = std::function<void(Event &)>;

	static std::unique_ptr<Window> Create(const WindowDesc &desc);

	virtual ~Window() = default;

	virtual void OnUpdate() = 0;
	virtual void WaitForEvents() = 0;

	void UpdateLayers(float ts);
	void RenderLayers();
	void OnEvent(Event &e);

	virtual bool ShouldClose() const = 0;

	virtual void SetTitle(std::string_view title) = 0;
	virtual std::string_view GetTitle() const = 0;

	virtual void SetExtent(uint32_t width, uint32_t height) = 0;
	virtual std::pair<uint32_t, uint32_t> GetExtent() const = 0;

	virtual void *GetNativeHandle() const = 0;
	virtual void *GetGLFWwindow() const = 0;
	virtual void* CreateVulkanSurface(void* instance) const = 0;

	void SetEventCallback(const EventCallbackFn &callback) { m_EventCallback = callback; }
	LayerStack &GetLayerStack() { return m_LayerStack; }

	// forward functions
	void PushLayer(std::shared_ptr<Layer> layer) { m_LayerStack.PushLayer(layer); }
	void PushOverlay(std::shared_ptr<Layer> overlay) { m_LayerStack.PushOverlay(overlay); }

	void PopLayer(const std::shared_ptr<Layer> &layer) { m_LayerStack.PopLayer(layer); }
	void PopOverlay(const std::shared_ptr<Layer> &overlay) { m_LayerStack.PopOverlay(overlay); }

	template <typename T, typename... Args>
		requires std::derived_from<T, Layer>
	std::shared_ptr<T> PushLayer(Args &&...args);

	template <typename T, typename... Args>
		requires std::derived_from<T, Layer>
	std::shared_ptr<T> PushOverlay(Args &&...args);

private:
	LayerStack m_LayerStack;
	EventCallbackFn m_EventCallback;
};

// Impl
template <typename T, typename... Args>
	requires std::derived_from<T, Layer>
inline std::shared_ptr<T> Window::PushLayer(Args &&...args) {
	return m_LayerStack.PushLayer<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
	requires std::derived_from<T, Layer>
inline std::shared_ptr<T> Window::PushOverlay(Args &&...args) {
	return m_LayerStack.PushOverlay<T>(std::forward<Args>(args)...);
}

} // namespace lm
