#include "renderer.hpp"

namespace lm {

Renderer::Renderer(const Window &window) : m_Window{ window } {
}

Renderer::~Renderer() {
}

void Renderer::OnRender() {
}

void Renderer::OnResize(WindowExtent extent) {
}

} // namespace lm
