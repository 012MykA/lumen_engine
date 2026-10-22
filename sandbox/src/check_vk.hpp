#pragma once

#include <volk.h>

#include <source_location>

namespace lmvk {

void CheckVk(VkResult result, std::source_location loc = std::source_location::current());

} // namespace lmvk

#define CHECK_VK(x) ::lmvk::CheckVk((x))
