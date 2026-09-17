#pragma once

#include "lumen_engine/windowing/window.hpp"

class GLFWwindow;

namespace lm {

class GlfwWindowDriver final : public Window {
public:
	GlfwWindowDriver(const WindowDesc &desc);
	~GlfwWindowDriver();

	void OnUpdate() override;
	void WaitForEvents() override;
	bool ShouldClose() const override;

	void SetTitle(std::string_view title) override;
	std::string_view GetTitle() const override;

	void SetExtent(WindowExtent extent) override;
	WindowExtent GetExtent() const override;

	void *GetNativeHandle() const override;
	void *GetGLFWwindow() const override;

private:
	void InitCallbacks();

private:
	GLFWwindow *m_Handle{ nullptr };
};

} // namespace lm
