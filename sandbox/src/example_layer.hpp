#pragma once

#include <lumen_engine/lumen_engine.hpp>

class ExampleLayer : public lm::Layer {
public:
	ExampleLayer();

	void OnEvent(lm::Event &e) override;
	void OnUpdate(float ts) override;
};
