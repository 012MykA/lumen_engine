#pragma once

// clang-format off
#include <volk.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
// clang-format on

#include <format>
#include <stdexcept>

#define CHECK_VK(expr) \
	do { \
		VkResult _res = (expr); \
		if (_res != VK_SUCCESS) { \
			throw std::runtime_error{ std::format("Vulkan error {} at {}:{} ({})", \
					string_VkResult(_res), __FILE__, __LINE__, #expr) }; \
		} \
	} while (0)
