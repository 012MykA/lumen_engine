#pragma once

#include "lumen_engine/core/layer.hpp"
#include "lumen_engine/core/layer_stack.hpp"
#include "lumen_engine/core/window.hpp"
#include "lumen_engine/gfx/graphics_context.hpp"
#include "lumen_engine/runtime/app_cmd_args.hpp"

#include <GLFW/glfw3.h>

#include <type_traits>

namespace lm {

class Application {
public:
	explicit Application(const ApplicationCmdArgs &args);
	~Application();

	static Application &Get();

public:
	void Run();
	void Close();

	template <typename T>
	void PushLayer(std::shared_ptr<T> layer);

	template <typename T, typename... Args>
		requires std::derived_from<T, Layer>
	void PushLayer(Args &&...args);

public:
	const ApplicationCmdArgs &GetArgs() const { return m_Args; }
	const Window &GetWindow() const { return m_Window; }

private:
	void Init();
	void Shutdown();

	void OnEvent(Event &e);

	static void InitGlfwCallbacks(GLFWwindow *handle);

private:
	ApplicationCmdArgs m_Args;
	bool m_Running{ true };

	Window m_Window;
	LayerStack m_LayerStack;

	GraphicsContext m_Graphics;

private:
	inline static Application *s_Instance{ nullptr };
};

std::unique_ptr<Application> CreateApplication(const ApplicationCmdArgs &args);

template <typename T>
inline void Application::PushLayer(std::shared_ptr<T> layer) {
	m_LayerStack.PushLayer(layer);
}

template <typename T, typename... Args>
	requires std::derived_from<T, Layer>
inline void Application::PushLayer(Args &&...args) {
	m_LayerStack.PushLayer<T>(std::forward<Args>(args)...);
}

} // namespace lm
