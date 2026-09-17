#pragma once

#include <vulkan/vk_enum_string_helper.h>

#include <format>
#include <stdexcept>

#define CHECK_VK(x) \
	do { \
		if ((VkResult)x != VK_SUCCESS) { \
			throw std::runtime_error(std::format("Vulkan error: {} at {}:{}", string_VkResult(x), __FILE__, __LINE__)); \
		} \
	} while (0)
