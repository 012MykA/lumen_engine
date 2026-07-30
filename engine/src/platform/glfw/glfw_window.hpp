#pragma once

#include "lumen_engine/core/window.hpp"

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

	void SetSize(WindowExtent extent) override;
	WindowExtent GetExtent() const override;

private:
	void InitCallbacks();

private:
	GLFWwindow *m_Handle{ nullptr };
};

} // namespace lm
