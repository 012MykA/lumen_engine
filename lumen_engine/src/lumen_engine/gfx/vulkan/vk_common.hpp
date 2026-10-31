#pragma once

// clang-format off
#include <volk.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
// clang-format on

#include <source_location>

namespace lm {

void CheckVk(VkResult result, std::source_location loc = std::source_location::current());

} // namespace lm

#define CHECK_VK(x) ::lm::CheckVk((x))
