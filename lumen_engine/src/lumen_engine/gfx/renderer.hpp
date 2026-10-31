#pragma once

#include "lumen_engine/core/window.hpp"

namespace lm {

class Renderer {
public:
	Renderer(const Window &window);
	~Renderer();

	void OnRender();

	void OnResize(WindowExtent extent);

private:
	const Window &m_Window;
};

} // namespace lm
