#pragma once

#include "lumen_engine/core/window.hpp"
#include "lumen_engine/gfx/vulkan/vk_context.hpp"

namespace lm {

class Renderer {
public:
	Renderer(const Window &window);
	~Renderer();

	void OnRender();

	void OnResize(WindowExtent extent);

private:
	const Window &m_Window;
	VulkanContext m_Context;
};

} // namespace lm
