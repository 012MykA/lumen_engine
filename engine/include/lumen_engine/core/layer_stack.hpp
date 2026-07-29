#pragma once

#include "lumen_engine/core/layer.hpp"

#include <vector>

namespace lm {

class LUMEN_API LayerStack {
public:
	LayerStack() = default;
	~LayerStack();

	void PushLayer(Layer *layer);
	void PushOverlay(Layer *overlay);

	void PopLayer(Layer *layer);
	void PopOverlay(Layer *overlay);

	auto begin() { return m_Layers.begin(); }
	auto end() { return m_Layers.end(); }
	auto rbegin() { return m_Layers.rbegin(); }
	auto rend() { return m_Layers.rend(); }

	auto begin() const { return m_Layers.begin(); }
	auto end() const { return m_Layers.end(); }
	auto rbegin() const { return m_Layers.rbegin(); }
	auto rend() const { return m_Layers.rend(); }

private:
	std::vector<Layer *> m_Layers;
	size_t m_LayerInsertIndex{ 0 };
};

} // namespace lm
