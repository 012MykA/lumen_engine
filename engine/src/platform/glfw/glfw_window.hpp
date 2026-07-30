#pragma once

#include "lumen_engine/core/window.hpp"

#include <GLFW/glfw3.h>

namespace lm {

class GlfwWindowDriver : public Window {
public:
	GlfwWindowDriver(const WindowDesc &desc);
	~GlfwWindowDriver();

	virtual void OnUpdate() override;
	virtual bool ShouldClose() const override;

	virtual void SetTitle(std::string_view title) override;
	virtual std::string_view GetTitle() const override;

	virtual void SetSize(WindowExtent extent) override;
	virtual WindowExtent GetExtent() const override;

private:
	void InitCallbacks();

private:
	GLFWwindow *m_Handle{ nullptr };
};

} // namespace lm
