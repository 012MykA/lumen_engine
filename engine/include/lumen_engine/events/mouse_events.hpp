#pragma once

#include "lumen_engine/events/event.hpp"
#include "lumen_engine/io/mouse_codes.hpp"

#include <format>
#include <type_traits> // for std::underlying_type
#include <utility> // for std::pair

namespace lm {

class MouseMovedEvent : public Event {
public:
	MouseMovedEvent(float xPos, float yPos) : m_PositionX{ xPos }, m_PositionY{ yPos } {}

	std::pair<float, float> GetPosition() const { return { m_PositionX, m_PositionY }; }
	float GetX() const { return m_PositionX; }
	float GetY() const { return m_PositionY; }

	std::string ToString() const override {
		return std::format("MouseMovedEvent: {}, {}", m_PositionX, m_PositionY);
	}

	EVENT_CLASS_TYPE(MouseMoved)
	EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)

public:
	float m_PositionX, m_PositionY;
};

class MouseScrolledEvent : public Event {
public:
	MouseScrolledEvent(float xOffset, float yOffset) : m_OffsetX{ xOffset }, m_OffsetY{ yOffset } {}

	std::pair<float, float> GetOffset() const { return { m_OffsetX, m_OffsetY }; }
	float GetX() const { return m_OffsetX; }
	float GetY() const { return m_OffsetY; }

	std::string ToString() const override {
		return std::format("MouseScrolledEvent: {}, {}", m_OffsetX, m_OffsetY);
	}

	EVENT_CLASS_TYPE(MouseScrolled)
	EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)

private:
	float m_OffsetX, m_OffsetY;
};

class MouseButtonEvent : public Event {
public:
	virtual ~MouseButtonEvent() = default;

	MouseCode GetButton() const { return m_Button; }

	EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)

protected:
	MouseButtonEvent(MouseCode button) : m_Button{ button } {}

protected:
	MouseCode m_Button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
	MouseButtonPressedEvent(MouseCode button) : MouseButtonEvent{ button } {}

	std::string ToString() const override {
		using T = std::underlying_type<MouseCode>;
		return std::format("MouseButtonPressedEvent: {}", static_cast<T>(m_Button));
	}

	EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
	MouseButtonReleasedEvent(MouseCode button) : MouseButtonEvent{ button } {}

	std::string ToString() const override {
		using T = std::underlying_type<MouseCode>;
		return std::format("MouseButtonReleasedEvent: {}", static_cast<T>(m_Button));
	}

	EVENT_CLASS_TYPE(MouseButtonReleased)
};

} // namespace lm
