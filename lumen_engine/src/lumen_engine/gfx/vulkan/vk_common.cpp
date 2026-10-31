#include "lumen_engine/gfx/vulkan/vk_common.hpp"

#include <format>
#include <stdexcept>

namespace lm {

void CheckVk(VkResult result, std::source_location loc) {
	if (result != VK_SUCCESS) {
		throw std::runtime_error(std::format("Vulkan error: {} at {}:{} ({})",
				string_VkResult(result), loc.file_name(), loc.line(), loc.function_name()));
	}
}

} // namespace lm
