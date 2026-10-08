#pragma once

#include <cstdint>
#include <string>

namespace lm {

struct WindowDesc {
	std::string title{ "Lumen Window" };
	uint32_t width{ 1280 }, height{ 720 };
	bool resizable{ true };
	bool focused{ true };
};

} // namespace lm
