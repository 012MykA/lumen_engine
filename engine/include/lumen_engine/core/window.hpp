#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/core/layer_stack.hpp"
#include "lumen_engine/events/event.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace lm {

struct WindowDesc {
	std::string title = "Lumen Engine";
	uint32_t width{ 1280 }, height{ 720 };
};

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
	virtual uint32_t GetWidth() const = 0;
	virtual uint32_t GetHeight() const = 0;
	virtual void *GetNativeWindow() const = 0;

	void SetEventCallback(const EventCallbackFn &callback) { m_EventCallback = callback; }
	LayerStack &GetLayerStack() { return m_LayerStack; }

private:
	LayerStack m_LayerStack;
	EventCallbackFn m_EventCallback;
};

} // namespace lm
