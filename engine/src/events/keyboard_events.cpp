#include "lumen_engine/events/keyboard_events.hpp"

#include <format>

namespace lm {

KeyEvent::KeyEvent(KeyCode key) : m_Key{ key } {}

KeyPressedEvent::KeyPressedEvent(KeyCode key, uint32_t repeatCount) : KeyEvent{ key }, m_RepeatCount{ repeatCount } {}

std::string KeyPressedEvent::ToString() const {
	return std::format("KeyPressedEvent: {}, repeatCount={}", m_Key, m_RepeatCount);
}

KeyReleasedEvent::KeyReleasedEvent(KeyCode key) : KeyEvent{ key } {}

std::string KeyReleasedEvent::ToString() const {
	return std::format("KeyReleasedEvent: {}", m_Key);
}

KeyTypedEvent::KeyTypedEvent(KeyCode key) : KeyEvent{ key } {}

std::string KeyTypedEvent::ToString() const {
	return std::format("KeyTypedEvent: {}", m_Key);
}

} // namespace lm
