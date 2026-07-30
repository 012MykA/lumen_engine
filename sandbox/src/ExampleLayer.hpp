#pragma once

#include <lumen_engine/lumen_engine.hpp>

class UIOverlay : public lm::Layer {
public:
	UIOverlay() : Layer("UIOverlay") {}

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnRender() override;
	virtual void OnUIRender() override;
};

class ExampleLayer : public lm::Layer {
public:
	ExampleLayer() : Layer("ExampleLayer") {}

	virtual void OnEvent(lm::Event &e) override;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnRender() override;
	virtual void OnUIRender() override;
};
