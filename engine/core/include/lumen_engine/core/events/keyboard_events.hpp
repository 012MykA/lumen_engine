#pragma once

#include "lumen_engine/core/key_codes.hpp"
#include "lumen_engine/core/events/event.hpp"

#include <cstdint>
#include <string>

namespace lm {

class KeyEvent : public Event {
public:
	virtual ~KeyEvent() = default;

	KeyCode GetKey() const { return m_Key; }

	EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

protected:
	KeyEvent(KeyCode key);

protected:
	KeyCode m_Key;
};

class KeyPressedEvent final : public KeyEvent {
public:
	KeyPressedEvent(KeyCode key, uint32_t repeatCount);

	uint32_t GetRepeatCount() const { return m_RepeatCount; }

	std::string ToString() const override;

	EVENT_CLASS_TYPE(KeyPressed)

private:
	uint32_t m_RepeatCount;
};

class KeyReleasedEvent final : public KeyEvent {
public:
	KeyReleasedEvent(KeyCode key);

	std::string ToString() const override;

	EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent final : public KeyEvent {
public:
	KeyTypedEvent(KeyCode key);

	std::string ToString() const override;

	EVENT_CLASS_TYPE(KeyTyped)
};

} // namespace lm
