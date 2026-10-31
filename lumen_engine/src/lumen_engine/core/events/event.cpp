#include "lumen_engine/core/events/event.hpp"

namespace lm {

bool Event::IsInCategory(EventCategory category) const {
	return HasCategory(GetCategoryFlags(), category);
}

} // namespace lm
