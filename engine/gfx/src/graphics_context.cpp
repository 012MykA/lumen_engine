#include "lumen_engine/gfx/graphics_context.hpp"

#include "lumen_engine/windowing/window.hpp"

#include <volk.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace lm {

namespace {

lmvk::InstanceDesc MakeInstanceDesc(const GraphicsContextDesc &d) {
	if (!glfwVulkanSupported()) {
		throw std::runtime_error("Vulkan is not supported by GLFW");
	}
	uint32_t n = 0;
	const char **exts = glfwGetRequiredInstanceExtensions(&n);
	return { .appName = d.appName, .enableValidation = d.enableValidation, .extraExtensions = { exts, exts + n } };
}

} //namespace

// clang-format off

GraphicsContext::GraphicsContext(const Window &target, const GraphicsContextDesc &desc) :
    m_Instance{MakeInstanceDesc(desc)},
	m_PhysicalDevice{m_Instance.Handle(),
		*reinterpret_cast<VkSurfaceKHR*>(target.CreateVulkanSurface(m_Instance.Handle())),
		lmvk::PhysicalDeviceDesc{
        .requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
        .presentSupport = [inst = m_Instance.Handle()] (VkPhysicalDevice pd, uint32_t family) {
			return glfwGetPhysicalDevicePresentationSupport(inst, pd, family) == GLFW_TRUE;
		}
	}},
	m_Device(m_PhysicalDevice),
	m_Allocator(m_Instance.Handle(), m_PhysicalDevice.Handle(), m_Device.Handle()) {}

// clang-format on

} // namespace lm
