#pragma once

#include "lumen_engine/core/export.hpp"

#include <cstdint>
#include <iostream>
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

constexpr EventCategory operator|(EventCategory a, EventCategory b) {
	using T = std::underlying_type_t<EventCategory>;
	return static_cast<EventCategory>(static_cast<T>(a) | static_cast<T>(b));
}

constexpr EventCategory operator&(EventCategory a, EventCategory b) {
	using T = std::underlying_type_t<EventCategory>;
	return static_cast<EventCategory>(static_cast<T>(a) & static_cast<T>(b));
}

constexpr EventCategory operator~(EventCategory e) {
	using T = std::underlying_type_t<EventCategory>;
	return static_cast<EventCategory>(~static_cast<T>(e));
}

constexpr bool HasCategory(EventCategory flags, EventCategory category) {
	return (flags & category) == category;
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

constexpr std::string_view EventTypeToString(EventType type) noexcept {
	using namespace std::string_view_literals;

	switch (type) { // clang-format off
        case EventType::None:                   return "None"sv;
        
		case EventType::WindowClose:            return "WindowClose"sv;
		case EventType::WindowResize:           return "WindowResize"sv;
		case EventType::WindowFocus:            return "WindowFocus"sv;
		case EventType::WindowLostFocus:        return "WindowLostFocus"sv;
		case EventType::WindowMoved:            return "WindowMoved"sv;

		case EventType::AppTick:                return "AppTick"sv;
		case EventType::AppUpdate:              return "AppUpdate"sv;
		case EventType::AppRender:              return "AppRender"sv;

		case EventType::KeyPressed:             return "KeyPressed"sv;
		case EventType::KeyReleased:            return "KeyReleased"sv;
		case EventType::KeyTyped:               return "KeyTyped"sv;

		case EventType::MouseMoved:             return "MouseMoved"sv;
		case EventType::MouseScrolled:          return "MouseScrolled"sv;
		case EventType::MouseButtonPressed:     return "MouseButtonPressed"sv;
		case EventType::MouseButtonReleased:    return "MouseButtonReleased"sv;
	} // clang-format on
	return "Unknown"sv;
}

class LUMEN_API Event {
public:
	virtual ~Event() = default;

	virtual EventType GetEventType() const = 0;
	virtual EventCategory GetCategoryFlags() const = 0;
	virtual std::string_view GetName() const = 0;
	virtual std::string ToString() const { return std::string(GetName()); }

	bool IsInCategory(EventCategory category) const;

public:
	bool handled{ false };
};

#define EVENT_CLASS_TYPE(type) \
	static EventType GetStaticType() { \
		return EventType::type; \
	} \
	virtual EventType GetEventType() const override { \
		return GetStaticType(); \
	} \
	virtual std::string_view GetName() const override { \
		return #type; \
	}

#define EVENT_CLASS_CATEGORY(category) \
	virtual EventCategory GetCategoryFlags() const override { \
		return EventCategory::category; \
	}

} // namespace lm
