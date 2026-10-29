#pragma once

#include "lumen_engine/core/enum_flags.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

namespace lm {

enum class EventCategory : uint32_t {
	None = 0,
	Application = 1 << 0,
	Window = 1 << 1,
	Input = 1 << 2,
	Keyboard = 1 << 3,
	Mouse = 1 << 4,
};

template <>
struct EnableEnumFlags<EventCategory> : std::true_type {};

constexpr bool HasCategory(EventCategory flags, EventCategory category) {
	return HasAnyFlag(flags, category);
}

enum class EventType {
	None = 0,

	AppTick,
	AppUpdate,
	AppRender,

	WindowClose,
	WindowResize,
	WindowFocus,
	WindowLostFocus,
	WindowMoved,

	KeyPressed,
	KeyReleased,
	KeyTyped,

	MouseMoved,
	MouseScrolled,
	MouseButtonPressed,
	MouseButtonReleased,
};

class Event {
public:
	virtual ~Event() = default;

	virtual EventType GetEventType() const = 0;
	virtual EventCategory GetCategoryFlags() const = 0;
	virtual std::string_view GetName() const = 0;
	virtual std::string ToString() const { return std::string{ GetName() }; }

	bool IsInCategory(EventCategory category) const;

public:
	bool handled{ false };
};

#define EVENT_CLASS_TYPE(type) \
	static EventType GetStaticType() { \
		return EventType::type; \
	} \
	EventType GetEventType() const override { \
		return GetStaticType(); \
	} \
	std::string_view GetName() const override { \
		return #type; \
	}

#define EVENT_CLASS_CATEGORY(category) \
	EventCategory GetCategoryFlags() const override { \
		return category; \
	}

} // namespace lm
