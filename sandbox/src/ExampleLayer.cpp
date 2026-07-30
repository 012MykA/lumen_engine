#include "ExampleLayer.hpp"

#include <print>

void ExampleLayer::OnEvent(lm::Event &e) {
	std::println("{}", e.ToString());

	if (e.IsInCategory(lm::EventCategory::Input)) {
		// TODO: key & mouse events;
		// std::println("Input handled: {}", e.ToString());
	}
}

void ExampleLayer::OnUpdate(float ts) {
	if (lm::Input::IsKeyPressed(lm::Key::W)) {
		std::println("W pressed!");
	}
}
