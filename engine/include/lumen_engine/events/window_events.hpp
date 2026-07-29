#pragma once

#include "lumen_engine/events/event.hpp"

#include <cstdint>
#include <format>

namespace lm {

class LUMEN_API WindowCloseEvent : public Event {
public:
	WindowCloseEvent() = default;

	EVENT_CLASS_TYPE(WindowClose)
	EVENT_CLASS_CATEGORY(Window)
};

class LUMEN_API WindowResizeEvent : public Event {
public:
	WindowResizeEvent(uint32_t width, uint32_t height) : m_Width{ width }, m_Height{ height } {}

	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }
	std::string ToString() const override {
		return std::format("WindowResize: {}, {}", m_Width, m_Height);
	}

	EVENT_CLASS_TYPE(WindowResize)
	EVENT_CLASS_CATEGORY(Window)

private:
	uint32_t m_Width, m_Height;
};

class LUMEN_API WindowFocusEvent : public Event {
public:
	WindowFocusEvent() = default;

	EVENT_CLASS_TYPE(WindowFocus)
	EVENT_CLASS_CATEGORY(Window)
};

class LUMEN_API WindowLostFocusEvent : public Event {
public:
	WindowLostFocusEvent() = default;

	EVENT_CLASS_TYPE(WindowLostFocus)
	EVENT_CLASS_CATEGORY(Window)
};

class LUMEN_API WindowMovedEvent : public Event {
public:
	WindowMovedEvent(int32_t x, int32_t y) : m_PositionX{ x }, m_PositionY{ y } {}

	int32_t GetX() const { return m_PositionX; }
	int32_t GetY() const { return m_PositionY; }
	virtual std::string ToString() const override {
		return std::format("WindowMoved: {}, {}", m_PositionX, m_PositionY);
	}

	EVENT_CLASS_TYPE(WindowMoved)
	EVENT_CLASS_CATEGORY(Window)

private:
	int32_t m_PositionX, m_PositionY;
};

} // namespace lm
