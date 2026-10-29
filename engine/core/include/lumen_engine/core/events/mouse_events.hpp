#pragma once

#include "lumen_engine/core/events/event.hpp"
#include "lumen_engine/core/input/mouse_codes.hpp"

#include <string>
#include <utility> // for std::pair

namespace lm {

class MouseMovedEvent final : public Event {
public:
	MouseMovedEvent(float xPos, float yPos);

	std::pair<float, float> GetPosition() const { return { m_PositionX, m_PositionY }; }
	float GetX() const { return m_PositionX; }
	float GetY() const { return m_PositionY; }

	std::string ToString() const override;

	EVENT_CLASS_TYPE(MouseMoved)
	EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)

private:
	float m_PositionX, m_PositionY;
};

class MouseScrolledEvent final : public Event {
public:
	MouseScrolledEvent(float xOffset, float yOffset);

	std::pair<float, float> GetOffset() const { return { m_OffsetX, m_OffsetY }; }
	float GetX() const { return m_OffsetX; }
	float GetY() const { return m_OffsetY; }

	std::string ToString() const override;

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
	MouseButtonEvent(MouseCode button);

protected:
	MouseCode m_Button;
};

class MouseButtonPressedEvent final : public MouseButtonEvent {
public:
	MouseButtonPressedEvent(MouseCode button);

	std::string ToString() const override;

	EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent final : public MouseButtonEvent {
public:
	MouseButtonReleasedEvent(MouseCode button);

	std::string ToString() const override;

	EVENT_CLASS_TYPE(MouseButtonReleased)
};

} // namespace lm
