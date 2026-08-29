#pragma once

#include <cstdint>

namespace lm {

using MouseCode = uint8_t;

namespace Mouse {

// From glfw3.h
enum : MouseCode {
	Button1 = 0,
	Button2 = 1,
	Button3 = 2,
	Button4 = 3,
	Button5 = 4,
	Button6 = 5,
	Button7 = 6,
	Button8 = 7,

	LeftButton = Button1,
	RightButton = Button2,
	ButtonMiddle = Button3,
};

} // namespace Mouse

} // namespace lm
