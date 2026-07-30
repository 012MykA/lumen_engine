#include "ExampleLayer.hpp"

#include <print>

void ExampleLayer::OnEvent(lm::Event &e) {
	std::println("{}", e.ToString());
}

void ExampleLayer::OnAttach() {
	std::println("ExampleLayer::OnAttach");
}

void ExampleLayer::OnDetach() {
	std::println("ExampleLayer::OnDetach");
}

void ExampleLayer::OnUpdate(float ts) {
	std::println("ExampleLayer::OnUpdate: {}", ts);
}

void ExampleLayer::OnRender() {
	std::println("ExampleLayer::OnRender");
}

void ExampleLayer::OnUIRender() {
	std::println("ExampleLayer::OnUIRender");
}

void UIOverlay::OnAttach() {
	std::println("UIOverlay::OnAttach");
}

void UIOverlay::OnDetach() {
	std::println("UIOverlay::OnDetach");
}

void UIOverlay::OnUpdate(float ts) {
	std::println("UIOverlay::OnUpdate: {}", ts);
}

void UIOverlay::OnRender() {
	std::println("UIOverlay::OnRender");
}

void UIOverlay::OnUIRender() {
	std::println("UIOverlay::OnUIRender");
}
