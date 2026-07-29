#pragma once

#include "lumen_engine/core/export.hpp"

#include <memory>
#include <string>
#include <vector>

namespace lm {

struct LUMEN_API ApplicationCommandLineArgs {
	ApplicationCommandLineArgs(int argc, char **argv) : args(argv, argv + argc) {}

	const std::string &operator[](size_t idx) const { return args[idx]; }

	std::vector<std::string> args;
};

class LUMEN_API Application {
public:
	explicit Application(const ApplicationCommandLineArgs &args);
	virtual ~Application();

	void Run();

private:
	ApplicationCommandLineArgs m_Args;

private:
	static inline Application *s_Instance{ nullptr };
};

std::unique_ptr<Application> CreateApplication(const ApplicationCommandLineArgs &args);

} // namespace lm
