#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/events/event.hpp"
#include "lumen_engine/io/key_codes.hpp"

#include <cstdint>
#include <string>

namespace lm {

class LUMEN_API KeyEvent : public Event {
public:
	virtual ~KeyEvent() = default;

	KeyCode GetKey() const { return m_Key; }

	EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

protected:
	KeyEvent(KeyCode key);

protected:
	KeyCode m_Key;
};

class LUMEN_API KeyPressedEvent : public KeyEvent {
public:
	KeyPressedEvent(KeyCode key, uint32_t repeatCount);

	uint32_t GetRepeatCount() const { return m_RepeatCount; }

	std::string ToString() const override;

	EVENT_CLASS_TYPE(KeyPressed)

private:
	uint32_t m_RepeatCount;
};

class LUMEN_API KeyReleasedEvent : public KeyEvent {
public:
	KeyReleasedEvent(KeyCode key);

	std::string ToString() const override;

	EVENT_CLASS_TYPE(KeyReleased)
};

class LUMEN_API KeyTypedEvent : public KeyEvent {
public:
	KeyTypedEvent(KeyCode key);

	std::string ToString() const override;

	EVENT_CLASS_TYPE(KeyTyped)
};

} // namespace lm
