#pragma once

#include "lumen_engine/core/events/event.hpp"

#include <cstdint>
#include <string>

namespace lm {

class WindowCloseEvent final : public Event {
public:
	WindowCloseEvent() = default;

	EVENT_CLASS_TYPE(WindowClose)
	EVENT_CLASS_CATEGORY(EventCategory::Window)
};

class WindowResizeEvent final : public Event {
public:
	WindowResizeEvent(uint32_t width, uint32_t height);

	std::pair<uint32_t, uint32_t> GetExtent() const { return { m_Width, m_Height }; }
	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }

	std::string ToString() const override;

	EVENT_CLASS_TYPE(WindowResize)
	EVENT_CLASS_CATEGORY(EventCategory::Window)

private:
	uint32_t m_Width, m_Height;
};

class WindowFocusEvent final : public Event {
public:
	WindowFocusEvent() = default;

	EVENT_CLASS_TYPE(WindowFocus)
	EVENT_CLASS_CATEGORY(EventCategory::Window)
};

class WindowLostFocusEvent final : public Event {
public:
	WindowLostFocusEvent() = default;

	EVENT_CLASS_TYPE(WindowLostFocus)
	EVENT_CLASS_CATEGORY(EventCategory::Window)
};

class WindowMovedEvent final : public Event {
public:
	WindowMovedEvent(int32_t x, int32_t y) : m_PositionX{ x }, m_PositionY{ y } {}

	std::pair<int32_t, int32_t> GetPosition() const { return { m_PositionX, m_PositionY }; }
	int32_t GetX() const { return m_PositionX; }
	int32_t GetY() const { return m_PositionY; }

	std::string ToString() const override;

	EVENT_CLASS_TYPE(WindowMoved)
	EVENT_CLASS_CATEGORY(EventCategory::Window)

private:
	int32_t m_PositionX, m_PositionY;
};

} // namespace lm
