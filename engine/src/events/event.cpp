#include "lumen_engine/events/event.hpp"

namespace lm {

bool Event::IsInCategory(EventCategory category) const {
	return GetCategoryFlags() & category;
}

} // namespace lm
