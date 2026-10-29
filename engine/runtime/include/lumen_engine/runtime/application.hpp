#pragma once

#include "lumen_engine/core/layer.hpp"
#include "lumen_engine/core/layer_stack.hpp"
#include "lumen_engine/core/window.hpp"
#include "lumen_engine/runtime/app_cmd_args.hpp"

#include <type_traits>

namespace lm {

class Application {
public:
	explicit Application(const ApplicationCmdArgs &args);
	~Application();

	Application(const Application &) = delete;
	Application &operator=(const Application &) = delete;

	static Application &Get();

public:
	void Run();
	void Close();

	template <typename T>
		requires std::derived_from<T, Layer>
	void PushLayer(std::shared_ptr<T> layer) {
		m_LayerStack.PushLayer(std::move(layer));
	}

	template <typename T, typename... Args>
		requires std::derived_from<T, Layer>
	void PushLayer(Args &&...args) {
		m_LayerStack.PushLayer<T>(std::forward<Args>(args)...);
	}

	template <typename T>
		requires std::derived_from<T, Layer>
	void PushOverlay(std::shared_ptr<T> overlay) {
		m_LayerStack.PushOverlay(std::move(overlay));
	}

	template <typename T, typename... Args>
		requires std::derived_from<T, Layer>
	void PushOverlay(Args &&...args) {
		m_LayerStack.PushOverlay<T>(std::forward<Args>(args)...);
	}

public:
	const ApplicationCmdArgs &GetArgs() const { return m_Args; }
	const Window &GetWindow() const { return m_Window; }

private:
	void OnEvent(Event &e);

	static void InitGlfwCallbacks(GLFWwindow *handle);

private:
	ApplicationCmdArgs m_Args;
	bool m_Running{ true };

	GlfwContext m_GlfwContext;
	Window m_Window;
	LayerStack m_LayerStack;

private:
	inline static Application *s_Instance{ nullptr };
};

std::unique_ptr<Application> CreateApplication(const ApplicationCmdArgs &args);

} // namespace lm
