#pragma once

#include <lumen_engine/lumen_engine.hpp>

class ExampleLayer : public lm::Layer {
public:
	ExampleLayer() : Layer("ExampleLayer") {}

	void OnEvent(lm::Event &e) override;
};
