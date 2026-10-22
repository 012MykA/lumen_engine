#pragma once

#include <lumen_engine/lumen_engine.hpp>

class RenderLayer : public lm::Layer {
public:
	RenderLayer() : Layer("RenderLayer") {}

	void OnRender() override {
		LUMEN_INFO("OnRender");
	}
};
