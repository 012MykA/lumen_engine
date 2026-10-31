#pragma once

#include <string>
#include <vector>

namespace lm {

class ApplicationCmdArgs {
public:
	ApplicationCmdArgs(int argc, char **argv) : args(argv, argv + argc) {}

	const std::string &operator[](size_t idx) const { return args[idx]; }
	const std::vector<std::string> &GetArgs() const { return args; }

private:
	std::vector<std::string> args;
};

} // namespace lm
