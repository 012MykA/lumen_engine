#pragma once

#include "lumen_engine/core/export.hpp"
#include "lumen_engine/core/window_desc.hpp"
#include "lumen_engine/events/event.hpp"

#include <cstdint>
#include <string>

namespace lm {

class LUMEN_API WindowCloseEvent final : public Event {
public:
	WindowCloseEvent() = default;

	EVENT_CLASS_TYPE(WindowClose)
	EVENT_CLASS_CATEGORY(EventCategory::Window)
};

class LUMEN_API WindowResizeEvent final : public Event {
public:
	WindowResizeEvent(WindowExtent extent);

	WindowExtent GetExtent() const { return m_Extent; }

	std::string ToString() const override;

	EVENT_CLASS_TYPE(WindowResize)
	EVENT_CLASS_CATEGORY(EventCategory::Window)

private:
	WindowExtent m_Extent;
};

class LUMEN_API WindowFocusEvent final : public Event {
public:
	WindowFocusEvent() = default;

	EVENT_CLASS_TYPE(WindowFocus)
	EVENT_CLASS_CATEGORY(EventCategory::Window)
};

class LUMEN_API WindowLostFocusEvent final : public Event {
public:
	WindowLostFocusEvent() = default;

	EVENT_CLASS_TYPE(WindowLostFocus)
	EVENT_CLASS_CATEGORY(EventCategory::Window)
};

class LUMEN_API WindowMovedEvent final : public Event {
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
