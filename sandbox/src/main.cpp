#include "lumen_engine/core/application.hpp"

#include <array>
#include <chrono>
#include <print>
#include <thread>

class Sandbox : public lm::Application {
public:
	explicit Sandbox(const lm::ApplicationCommandLineArgs &args) : lm::Application(args) {}

	void run() override {
		constexpr std::array frames = { '|', '/', '-', '\\' };
		size_t frameIndex{};

		while (true) {
			std::print("\r{} Running", frames[frameIndex]);
			std::fflush(stdout);

			frameIndex = (frameIndex + 1) % frames.size();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
};

int main(int argc, char *argv[]) {
	try {
		constexpr std::string_view buildType =
#if defined(LUMEN_DYNAMIC_LINK)
				"Dynamic Library";
#elif defined(LUMEN_STATIC_LINK)
				"Static Library";
#else
				"Unknown";
#endif

		std::print("Built as: {}\n", buildType);

		Sandbox app({ argc, argv });
		app.run();

	} catch (const std::exception &e) {
		std::println(stderr, "Error: {}", e.what());
	}
}
