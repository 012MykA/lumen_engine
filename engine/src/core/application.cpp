#include "lumen_engine/core/application.hpp"

#include <array>
#include <cassert>
#include <chrono>
#include <print>
#include <thread>

namespace lm {

Application::Application(const ApplicationCommandLineArgs &args) : m_Args(args) {
	assert(!s_Instance && "Application already exists!");
	s_Instance = this;
}

Application::~Application() {
	s_Instance = nullptr;
}

void Application::Run() {
	constexpr std::array frames = { '|', '/', '-', '\\' };
	size_t frame_index = 0;

	while (true) {
		std::print("\r{} Running", frames[frame_index]);
		std::fflush(stdout);

		frame_index = (frame_index + 1) % frames.size();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

} // namespace lm
