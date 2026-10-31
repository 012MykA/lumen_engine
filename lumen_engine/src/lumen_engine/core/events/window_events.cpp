#include "lumen_engine/core/events/window_events.hpp"

#include <format>

namespace lm {

WindowResizeEvent::WindowResizeEvent(uint32_t width, uint32_t height) : m_Width{ width }, m_Height{ height } {}

std::string WindowResizeEvent::ToString() const {
	return std::format("WindowResize: {}, {}", m_Width, m_Height);
}

std::string WindowMovedEvent::ToString() const {
	return std::format("WindowMoved: {}, {}", m_PositionX, m_PositionY);
}

} // namespace lm
