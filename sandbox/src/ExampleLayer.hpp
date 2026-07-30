#pragma once

#include <lumen_engine/lumen_engine.hpp>

class ExampleLayer : public lm::Layer {
public:
	ExampleLayer() : Layer("ExampleLayer") {}

	virtual void OnEvent(lm::Event &e) override;
};
