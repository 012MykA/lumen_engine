#pragma once

#include "lumen_engine/core/enum_flags.hpp"

#include <cstdint>

namespace lm {

enum class Modifiers : uint8_t {
	None = 0,
	Shift = 1 << 0,
	Control = 1 << 1,
	Alt = 1 << 2,
	Super = 1 << 3,
	CapsLock = 1 << 4,
	NumLock = 1 << 5,

	Ctrl = Control,
};

template <>
struct EnableEnumFlags<Modifiers> : std::true_type {};

} // namespace lm
