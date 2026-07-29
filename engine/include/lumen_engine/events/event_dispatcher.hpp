#pragma once

#include "lumen_engine/events/event.hpp"

#include <type_traits>

namespace lm {

class EventDispatcher {
	template <typename T>
	using EventFn = std::function<bool(T &)>;

public:
	explicit EventDispatcher(Event &e) : m_Event{ e } {}

	template <typename T>
		requires std::derived_from<T, Event>
	bool Dispatch(EventFn<T> func) {
		if (m_Event.GetEventType() == T::GetStaticType()) {
			m_Event.handled |= func(*static_cast<T *>(&m_Event));
			return true;
		}
		return false;
	}

private:
	Event &m_Event;
};

} // namespace lm
