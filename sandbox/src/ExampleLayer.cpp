#include "ExampleLayer.hpp"

#include <print>

ExampleLayer::ExampleLayer() : Layer("ExampleLayer") {}

void ExampleLayer::OnEvent(lm::Event &e) {
	if (e.GetEventType() == lm::EventType::MouseScrolled) {
		std::println("Mouse scrolled: {}", e.ToString());
	}
}

void ExampleLayer::OnUpdate(float ts) {}
