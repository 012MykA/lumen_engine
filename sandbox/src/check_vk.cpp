#include "check_vk.hpp"

#include <vulkan/vk_enum_string_helper.h>

#include <format>
#include <stdexcept>

namespace lmvk {

void CheckVk(VkResult result, std::source_location loc) {
	if (result != VK_SUCCESS) {
		throw std::runtime_error(std::format("Vulkan error: {} at {}:{} ({})",
				string_VkResult(result), loc.file_name(), loc.line(), loc.function_name()));
	}
}

} // namespace lmvk
