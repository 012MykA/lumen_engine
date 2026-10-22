#include "lumen_engine/gfx/graphics_context.hpp"

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

GraphicsContext::GraphicsContext(const GraphicsContextDesc &desc) {}

GraphicsContext::~GraphicsContext() {}

} // namespace lm
