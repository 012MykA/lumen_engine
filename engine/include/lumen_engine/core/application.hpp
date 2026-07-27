#pragma once

#include "lumen_engine/core/export.hpp"

#include <cassert>
#include <memory>
#include <string_view>
#include <vector>

namespace lm {

struct LUMEN_API ApplicationCommandLineArgs {
	ApplicationCommandLineArgs(int argc, char **argv) : args(argv, argv + argc) {}

	std::vector<std::string_view> args;
	std::string_view operator[](size_t idx) const { return args[idx]; }
};

class LUMEN_API Application {
public:
	explicit Application(const ApplicationCommandLineArgs &args);
	virtual ~Application() = default;

	virtual void run() = 0;

private:
	static inline Application *s_Instance = nullptr;
};

std::unique_ptr<Application> CreateApplication(ApplicationCommandLineArgs args);

} // namespace lm
