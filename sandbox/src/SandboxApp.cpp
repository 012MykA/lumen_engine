#include "lumen_engine/core/application.hpp"
#include "lumen_engine/core/entry_point.hpp"

#include <print>

class SandboxApp : public lm::Application {
public:
	explicit SandboxApp(const lm::ApplicationCommandLineArgs &args) : lm::Application(args) {}
};

std::unique_ptr<lm::Application> lm::CreateApplication(const lm::ApplicationCommandLineArgs &args) {
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
