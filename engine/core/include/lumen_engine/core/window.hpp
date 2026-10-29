#pragma once

#include "lumen_engine/core/log.hpp"

#include <cstdint>
#include <string>

struct GLFWwindow;

namespace lm {

class GlfwContext {
public:
	GlfwContext();
	~GlfwContext();

	GlfwContext(const GlfwContext &) = delete;
	GlfwContext &operator=(const GlfwContext &) = delete;
	GlfwContext(GlfwContext &&) = delete;
	GlfwContext &operator=(GlfwContext &&) = delete;

	static GlfwContext &Get();

private:
	inline static GlfwContext *s_Instance{ nullptr };
};

struct WindowExtent {
	uint32_t width{ 0 }, height{ 0 };
};

struct WindowDesc {
	uint32_t width{ 1280 }, height{ 720 };
	std::string title{ "Lumen Engine" };
};

struct Window {
	GLFWwindow *handle{ nullptr };

	Window() = default;
	~Window();
	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;

	void Create(const WindowDesc &desc = {});
	void Destroy();

	WindowExtent GetExtent() const;
};

} // namespace lm
