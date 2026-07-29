#pragma once

#include "lumen_engine/core/export.hpp"

#include <cstdint>
#include <iostream>
#include <string_view>

namespace lm {

enum EventCategory : uint32_t {
	EventCategoryNone = 0,
	EventCategoryApplication = 1 << 0,
	EventCategoryInput = 1 << 1,
	EventCategoryKeyboard = 1 << 2,
	EventCategoryMouse = 1 << 3,
	EventCategoryMouseButton = 1 << 4,
};

inline EventCategory operator|(EventCategory a, EventCategory b) {
	return static_cast<EventCategory>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline EventCategory operator&(EventCategory a, EventCategory b) {
	return static_cast<EventCategory>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline EventCategory operator~(EventCategory e) {
	return static_cast<EventCategory>(~static_cast<uint32_t>(e));
}

enum class EventType {
	None = 0,

	WindowClose,
	WindowResize,
	WindowFocus,
	WindowLostFocus,
	WindowMoved,

	AppTick,
	AppUpdate,
	AppRender,

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
	virtual uint32_t GetCategoryFlags() const = 0;

	bool IsInCategory(EventCategory category) const;

public:
	bool handled{ false };
};

#define EVENT_CLASS_TYPE(type) \
	static EventType GetStaticType() { \
		return EventType::type; \
	} \
	virtual EventType GetEventType() const override { \
		return EventType::type; \
	}

#define EVENT_CLASS_CATEGORY(category) \
	virtual uint32_t GetCategoryFlags() const override { \
		return category; \
	}

} // namespace lm
