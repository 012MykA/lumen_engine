#include "core/application.hpp"

#include <print>

namespace lm {

Application::Application(const ApplicationCommandLineArgs &args) {
	std::println("Lumen Application created!");
}

} // namespace lm
