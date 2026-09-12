#pragma once

#include <vulkan/vk_enum_string_helper.h>

#include <format>
#include <stdexcept>

#ifndef CHECK_VK(x)
	#define CHECK_VK(x) \
		do { \
			VkResult res = x; \
			if (res != VK_SUCCESS) { \
				throw std::runtime_error(std::format("Vulkan error: {} at {}:{}", string_VkResult(res), __FILE__, __LINE__)); \
			} \
		} while (0)
#endif
