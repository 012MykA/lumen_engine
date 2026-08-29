#pragma once

#include "lumen_engine/windowing/window.hpp"

#include <GLFW/glfw3.h>

namespace lm {

class GlfwWindowDriver : public Window {
public:
	GlfwWindowDriver(const WindowDesc &desc);
	~GlfwWindowDriver();

	void OnUpdate() override;
	bool ShouldClose() const override;

	void SetTitle(std::string_view title) override;
	std::string_view GetTitle() const override;

	void SetExtent(WindowExtent extent) override;
	WindowExtent GetExtent() const override;

	void *GetNativeWindow() const override { return m_Handle; }

private:
	void InitCallbacks();

private:
	GLFWwindow *m_Handle{ nullptr };
};

} // namespace lm
