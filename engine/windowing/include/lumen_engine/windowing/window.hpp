#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/core/layer_stack.hpp"
#include "lumen_engine/core/window_desc.hpp"
#include "lumen_engine/events/event.hpp"

#include <functional>
#include <memory>
#include <string_view>

namespace lm {

class LUMEN_API Window {
public:
	using EventCallbackFn = std::function<void(Event &)>;

	static std::unique_ptr<Window> Create(const WindowDesc &desc);

	virtual ~Window() = default;

	virtual void OnUpdate() = 0;
	void UpdateLayers(float ts);
	void RenderLayers();
	void OnEvent(Event &e);

	virtual bool ShouldClose() const = 0;

	virtual void SetTitle(std::string_view title) = 0;
	virtual std::string_view GetTitle() const = 0;

	virtual void SetExtent(WindowExtent extent) = 0;
	virtual WindowExtent GetExtent() const = 0;

	virtual void *GetNativeWindow() const = 0;

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
