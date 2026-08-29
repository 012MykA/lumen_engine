#pragma once

#include <cstdint>
#include <string>

namespace lm {

struct WindowExtent {
	uint32_t width{}, height{};
};

struct WindowDesc {
	std::string title = "Lumen Engine";
	WindowExtent extent{ 1280, 720 };
	bool resizable{ true };
	bool focused{ true };
};

} // namespace lm
