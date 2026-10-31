#pragma once

#include "lumen_engine/core/events/event.hpp"
#include "lumen_engine/core/input/key_codes.hpp"
#include "lumen_engine/core/input/modifiers.hpp"

#include <cstdint>
#include <string>

namespace lm {

class KeyEvent : public Event {
public:
	virtual ~KeyEvent() = default;

	KeyCode GetKey() const { return m_Key; }
	Modifiers GetModifiers() const { return m_Mods; }
	bool HasModifier(Modifiers mod) const { return HasAllFlags(m_Mods, mod); }

	EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

protected:
	KeyEvent(KeyCode key, Modifiers mods);

protected:
	KeyCode m_Key;
	Modifiers m_Mods;
};

class KeyPressedEvent final : public KeyEvent {
public:
	KeyPressedEvent(KeyCode key, uint32_t repeatCount, Modifiers mods = Modifiers::None);

	uint32_t GetRepeatCount() const { return m_RepeatCount; }

	std::string ToString() const override;

	EVENT_CLASS_TYPE(KeyPressed)

private:
	uint32_t m_RepeatCount;
};

class KeyReleasedEvent final : public KeyEvent {
public:
	KeyReleasedEvent(KeyCode key, Modifiers mods = Modifiers::None);

	std::string ToString() const override;

	EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent final : public Event {
public:
	explicit KeyTypedEvent(char32_t codepoint);

	char32_t GetCodePoint() const { return m_CodePoint; }

	std::string ToString() const override;

	EVENT_CLASS_TYPE(KeyTyped)
	EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

private:
	char32_t m_CodePoint;
};

} // namespace lm
