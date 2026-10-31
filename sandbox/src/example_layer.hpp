#pragma once

#include <lumen_engine.hpp>

class ExampleLayer : public lm::Layer {
public:
	ExampleLayer();

	void OnEvent(lm::Event &event) override;
	// void OnUpdate(float /*ts*/) override;
	void OnRender() override;
	// void OnUIRender() override;

private:
	bool OnWindowResize(lm::WindowResizeEvent &e);

private:
	lm::Renderer m_Renderer;
};
