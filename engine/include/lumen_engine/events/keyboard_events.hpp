#pragma once

#include "lumen_engine/events/event.hpp"
#include "lumen_engine/io/key_codes.hpp"

#include <cstdint>
#include <format>
#include <type_traits> // std::underlying_type

namespace lm {

class KeyEvent : public Event {
public:
	virtual ~KeyEvent() = default;

	KeyCode GetKey() const { return m_Key; }

	EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

protected:
	KeyEvent(Keycode key) : m_Key{ key } {}

protected:
	KeyCode m_Key;
};

class KeyPressedEvent : public KeyEvent {
public:
	KeyPressedEvent(KeyCode key, uint32_t repeatCount) : KeyEvent{ key }, m_RepeatCount{ repeatCount } {}

	uint32_t GetRepeatCount() const { return m_RepeatCount; }

	std::string ToString() const override {
		using T = std::underlying_type<KeyCode>;
		return std::format("KeyPressedEvent: {}", static_cast<T>(m_Key));
	}

	EVENT_CLASS_TYPE(KeyPressed);

private:
	uint32_t m_RepeatCount;
};

class KeyReleasedEvent : public KeyEvent {
public:
	KeyPressedEvent(KeyCode key) : KeyEvent{ key } {}

	std::string ToString() const override {
		using T = std::underlying_type<KeyCode>;
		return std::format("KeyReleasedEvent: {}", static_cast<T>(m_Key));
	}

	EVENT_CLASS_TYPE(KeyReleased);
};

class KeyTypedEvent : public KeyEvent {
public:
	KeyTypedEvent(KeyCode key) : KeyEvent{ key } {}

	std::string ToString() const override {
		using T = std::underlying_type<KeyCode>;
		return std::format("KeyTypedEvent: {}", static_cast<T>(m_Key));
	}

	EVENT_CLASS_TYPE(KeyTyped);
};

} // namespace lm
