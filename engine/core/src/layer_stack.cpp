#include "lumen_engine/core/layer_stack.hpp"

#include <algorithm>

namespace lm {

LayerStack::~LayerStack() {
	for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
		(*it)->OnDetach();
	}
}

void LayerStack::PushLayer(std::shared_ptr<Layer> layer) {
	layer->OnAttach();
	m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, std::move(layer));
	m_LayerInsertIndex++;
}

void LayerStack::PushOverlay(std::shared_ptr<Layer> overlay) {
	overlay->OnAttach();
	m_Layers.emplace_back(std::move(overlay));
}

void LayerStack::PopLayer(const std::shared_ptr<Layer> &layer) {
	auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
	if (it != m_Layers.begin() + m_LayerInsertIndex) {
		layer->OnDetach();
		m_Layers.erase(it);
		m_LayerInsertIndex--;
	}
}

void LayerStack::PopOverlay(const std::shared_ptr<Layer> &overlay) {
	auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
	if (it != m_Layers.end()) {
		overlay->OnDetach();
		m_Layers.erase(it);
	}
}

} // namespace lm
