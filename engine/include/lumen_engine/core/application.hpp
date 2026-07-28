#pragma once

#include "lumen_engine/core/export.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace lm {

struct LUMEN_API ApplicationCommandLineArgs {
	std::vector<std::string> args;

	ApplicationCommandLineArgs(int argc, char **argv) : args(argv, argv + argc) {}

	const std::string &operator[](size_t idx) const { return args[idx]; }
};

class LUMEN_API Application {
public:
	explicit Application(const ApplicationCommandLineArgs &args);
	virtual ~Application();

	virtual void run() = 0;

private:
	ApplicationCommandLineArgs m_Args;

private:
	static inline Application *s_Instance = nullptr;
};

std::unique_ptr<Application> CreateApplication(const ApplicationCommandLineArgs &args);

} // namespace lm
