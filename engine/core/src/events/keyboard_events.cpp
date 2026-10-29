#include "lumen_engine/core/events/keyboard_events.hpp"

#include <format>

namespace lm {

KeyEvent::KeyEvent(KeyCode key, Modifiers mods) : m_Key{ key }, m_Mods{ mods } {}

KeyPressedEvent::KeyPressedEvent(KeyCode key, uint32_t repeatCount, Modifiers mods) : KeyEvent{ key, mods },
																					  m_RepeatCount{ repeatCount } {}

std::string KeyPressedEvent::ToString() const {
	return std::format("KeyPressedEvent: {}, repeatCount={}, mods={:#04x}", m_Key, m_RepeatCount, static_cast<uint32_t>(m_Mods));
}

KeyReleasedEvent::KeyReleasedEvent(KeyCode key, Modifiers mods) : KeyEvent{ key, mods } {}

std::string KeyReleasedEvent::ToString() const {
	return std::format("KeyReleasedEvent: {}, mods={:#04x}", m_Key, static_cast<uint32_t>(m_Mods));
}

KeyTypedEvent::KeyTypedEvent(char32_t codepoint) : m_CodePoint{ codepoint } {}

std::string KeyTypedEvent::ToString() const {
	return std::format("KeyTypedEvent: U+{:04X}", static_cast<uint32_t>(m_CodePoint));
}

} // namespace lm
