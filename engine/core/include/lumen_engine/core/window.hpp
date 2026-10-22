#pragma once

#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

namespace lm {

struct WindowExtent {
	uint32_t width{ 0 }, height{ 0 };
};

struct WindowDesc {
	uint32_t width{ 1280 }, height{ 720 };
	std::string title{ "Lumen Engine" };
};

struct Window {
	GLFWwindow *handle{ nullptr };

	void Create(const WindowDesc &desc = {});
	void Destroy();

	WindowExtent GetExtent() const;
};

} // namespace lm
