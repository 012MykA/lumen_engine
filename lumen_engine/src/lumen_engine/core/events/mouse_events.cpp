#include "lumen_engine/core/events/mouse_events.hpp"

#include <format>

namespace lm {
MouseMovedEvent::MouseMovedEvent(float xPos, float yPos) : m_PositionX{ xPos }, m_PositionY{ yPos } {}

std::string MouseMovedEvent::ToString() const {
	return std::format("MouseMovedEvent: {}, {}", m_PositionX, m_PositionY);
}

MouseScrolledEvent::MouseScrolledEvent(float xOffset, float yOffset) : m_OffsetX{ xOffset }, m_OffsetY{ yOffset } {}

std::string MouseScrolledEvent::ToString() const {
	return std::format("MouseScrolledEvent: {}, {}", m_OffsetX, m_OffsetY);
}

MouseButtonEvent::MouseButtonEvent(MouseCode button) : m_Button{ button } {}

MouseButtonPressedEvent::MouseButtonPressedEvent(MouseCode button) : MouseButtonEvent{ button } {}

std::string MouseButtonPressedEvent::ToString() const {
	return std::format("MouseButtonPressedEvent: {}", m_Button);
}

MouseButtonReleasedEvent::MouseButtonReleasedEvent(MouseCode button) : MouseButtonEvent{ button } {}

std::string MouseButtonReleasedEvent::ToString() const {
	return std::format("MouseButtonReleasedEvent: {}", m_Button);
}

} // namespace lm
