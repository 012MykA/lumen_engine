#include "lumen_engine/core/application.hpp"

#include <cassert>

namespace lm {

Application::Application(const ApplicationCommandLineArgs &args) : m_Args(args) {
	assert(!s_Instance && "Application already exists!");
	s_Instance = this;
}

Application::~Application() {
	s_Instance = nullptr;
}

} // namespace lm
