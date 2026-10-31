#pragma once

#include <concepts>
#include <type_traits>

namespace lm {

/*
--- 'EnableEnumFlags' Usage Example ---

enum class Category : uint8_t {
	Category1 = 0,
	Category2,
	Category3,
};

// Define an overload for EnableEnumFlags struct

template<>
struct EnableEnumFlags<Category> : std::true_type {};

Now 'Category' type supports operators: |, &, ^, ~, |=, &=, ^=

*/

template <typename E>
struct EnableEnumFlags : std::false_type {};

template <typename E>
concept EnumFlags = std::is_enum_v<E> && EnableEnumFlags<E>::value;

// -------------------
// --- Flags check ---
// -------------------

template <EnumFlags E>
constexpr bool HasAllFlags(E value, E flag) {
	return (value & flag) == flag;
}

template <EnumFlags E>
constexpr bool HasAnyFlag(E value, E flag) {
	using T = std::underlying_type_t<E>;
	return static_cast<T>(value & flag) != 0;
}

// --------------------------
// --- Operators overload ---
// --------------------------

// lumen_engine\src\lumen_engine\core\enum_flags.hpp:
template <EnumFlags E>
constexpr E operator|(E a, E b) {
	using T = std::underlying_type_t<E>;
	return static_cast<E>(static_cast<T>(a) | static_cast<T>(b));
}

template <EnumFlags E>
constexpr E operator&(E a, E b) {
	using T = std::underlying_type_t<E>;
	return static_cast<E>(static_cast<T>(a) & static_cast<T>(b));
}

template <EnumFlags E>
constexpr E operator^(E a, E b) {
	using T = std::underlying_type_t<E>;
	return static_cast<E>(static_cast<T>(a) ^ static_cast<T>(b));
}

template <EnumFlags E>
constexpr E operator~(E a) {
	using T = std::underlying_type_t<E>;
	return static_cast<E>(~static_cast<T>(a));
}

template <EnumFlags E>
constexpr E &operator|=(E &a, E b) {
	return a = a | b;
}

template <EnumFlags E>
constexpr E &operator&=(E &a, E b) {
	return a = a & b;
}

template <EnumFlags E>
constexpr E &operator^=(E &a, E b) {
	return a = a ^ b;
}

} // namespace lm
