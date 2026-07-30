#pragma once

#include "lumen_engine/events/event.hpp"
#include "lumen_engine/io/key_codes.hpp"

#include <cstdint>
#include <format>

namespace lm {

class KeyEvent : public Event {
public:
	virtual ~KeyEvent() = default;

	KeyCode GetKey() const { return m_Key; }

	EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

protected:
	KeyEvent(KeyCode key) : m_Key{ key } {}

protected:
	KeyCode m_Key;
};

class KeyPressedEvent : public KeyEvent {
public:
	KeyPressedEvent(KeyCode key, uint32_t repeatCount) : KeyEvent{ key }, m_RepeatCount{ repeatCount } {}

	uint32_t GetRepeatCount() const { return m_RepeatCount; }

	std::string ToString() const override {
		return std::format("KeyPressedEvent: {}, repeatCount={}", m_Key, m_RepeatCount);
	}

	EVENT_CLASS_TYPE(KeyPressed);

private:
	uint32_t m_RepeatCount;
};

class KeyReleasedEvent : public KeyEvent {
public:
	KeyReleasedEvent(KeyCode key) : KeyEvent{ key } {}

	std::string ToString() const override {
		return std::format("KeyReleasedEvent: {}", m_Key);
	}

	EVENT_CLASS_TYPE(KeyReleased);
};

class KeyTypedEvent : public KeyEvent {
public:
	KeyTypedEvent(KeyCode key) : KeyEvent{ key } {}

	std::string ToString() const override {
		return std::format("KeyTypedEvent: {}", m_Key);
	}

	EVENT_CLASS_TYPE(KeyTyped);
};

} // namespace lm
