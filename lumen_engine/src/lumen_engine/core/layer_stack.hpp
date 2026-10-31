#pragma once

#include "lumen_engine/core/layer.hpp"

#include <concepts>
#include <memory>
#include <vector>

namespace lm {

class LayerStack final {
public:
	LayerStack() = default;
	~LayerStack();

	void PushLayer(std::shared_ptr<Layer> layer);
	void PushOverlay(std::shared_ptr<Layer> overlay);

	void PopLayer(const std::shared_ptr<Layer> &layer);
	void PopOverlay(const std::shared_ptr<Layer> &overlay);

	void Clear();

	template <typename T, typename... Args>
		requires std::derived_from<T, Layer>
	std::shared_ptr<T> PushLayer(Args &&...args) {
		auto layer = std::make_shared<T>(std::forward<Args>(args)...);
		PushLayer(std::static_pointer_cast<Layer>(layer));
		return layer;
	}

	template <typename T, typename... Args>
		requires std::derived_from<T, Layer>
	std::shared_ptr<T> PushOverlay(Args &&...args) {
		auto overlay = std::make_shared<T>(std::forward<Args>(args)...);
		PushOverlay(overlay);
		return overlay;
	}

	auto begin() { return m_Layers.begin(); }
	auto end() { return m_Layers.end(); }
	auto rbegin() { return m_Layers.rbegin(); }
	auto rend() { return m_Layers.rend(); }

	auto begin() const { return m_Layers.begin(); }
	auto end() const { return m_Layers.end(); }
	auto rbegin() const { return m_Layers.rbegin(); }
	auto rend() const { return m_Layers.rend(); }

private:
	std::vector<std::shared_ptr<Layer>> m_Layers;
	size_t m_LayerInsertIndex{ 0 };
};

} // namespace lm
