#include "ExampleLayer.hpp"

#include <print>

void ExampleLayer::OnEvent(lm::Event &e) {
	std::println("{}", e.ToString());
}
