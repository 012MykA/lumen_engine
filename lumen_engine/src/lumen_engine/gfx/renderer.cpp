#include "lumen_engine/gfx/renderer.hpp"

namespace lm {

Renderer::Renderer(const Window &window) :
		m_Window{ window },
		m_Context{ m_Window },
		m_Swapchain{ m_Context } {
}

Renderer::~Renderer() {
}

void Renderer::OnRender() {
}

void Renderer::OnResize(WindowExtent extent) {
	if (extent.width != 0 && extent.height != 0) {
		m_Swapchain.Recreate();
	}
}

} // namespace lm
