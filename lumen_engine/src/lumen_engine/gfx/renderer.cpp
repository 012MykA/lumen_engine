#include "lumen_engine/gfx/renderer.hpp"

namespace lm {

Renderer::Renderer(const Window &window) :
		m_Window{ window },
		m_Context{ m_Window } {
}

Renderer::~Renderer() {
}

void Renderer::OnRender() {
}

void Renderer::OnResize(WindowExtent extent) {
}

} // namespace lm
