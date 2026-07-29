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

	virtual uint32_t GetWidth() const override { return m_Width; }
	virtual uint32_t GetHeight() const override { return m_Height; }
	virtual void *GetNativeWindow() const override { return m_Handle; }

private:
	void InitCallbacks();

private:
	GLFWwindow *m_Handle{ nullptr };
	uint32_t m_Width, m_Height;
};

} // namespace lm
