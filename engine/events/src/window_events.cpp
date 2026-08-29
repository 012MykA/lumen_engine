#include "lumen_engine/events/window_events.hpp"

#include <format>

namespace lm {

WindowResizeEvent::WindowResizeEvent(WindowExtent extent) : m_Extent{ extent } {}

std::string WindowResizeEvent::ToString() const {
	return std::format("WindowResize: {}, {}", m_Extent.width, m_Extent.height);
}

std::string WindowMovedEvent::ToString() const {
	return std::format("WindowMoved: {}, {}", m_PositionX, m_PositionY);
}

} // namespace lm
