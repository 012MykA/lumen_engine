#include "lumen_engine/core/application.hpp"
#include "lumen_engine/core/entry_point.hpp"

#include <array>
#include <chrono>
#include <print>
#include <thread>

class SandboxApp : public lm::Application {
public:
	explicit SandboxApp(const lm::ApplicationCommandLineArgs &args) : lm::Application(args) {}

	virtual void run() override {
		constexpr std::array frames = { '|', '/', '-', '\\' };
		size_t frame_index = 0;
		while (true) {
			std::print("\r{} Running", frames[frame_index]);
			std::fflush(stdout);

			frame_index = (frame_index + 1) % frames.size();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
};

std::unique_ptr<lm::Application> lm::CreateApplication(lm::ApplicationCommandLineArgs args) {
	constexpr std::string_view buildType =
#if defined(LUMEN_DYNAMIC_LINK)
			"Dynamic Library";
#elif defined(LUMEN_STATIC_LINK)
			"Static Library";
#else
			"Unknown";
#endif
	std::print("Built as: {}\n", buildType);

	return std::make_unique<SandboxApp>(args);
}
