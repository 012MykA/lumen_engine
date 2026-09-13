#include "lumen_engine/rhi/vulkan/vulkan_device.hpp"
#include "vulkan_validation.hpp"

namespace lm::rhi {

void VulkanDevice::CreateSurface(void *window) {
#if defined(LUMEN_PLATFORM_WINDOWS)
	HWND handle = static_cast<HWND>(window);
	const VkWin32SurfaceCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hwnd = handle,
	};
	CHECK_VK(vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface));
#else
	#error Unsupported platform
#endif
}

} // namespace lm::rhi
