#pragma once

#include "lumen_engine/core/application.hpp"
#include "lumen_engine/core/export.hpp"

#include <iostream>
#include <print>
#include <stdexcept>

namespace lm {

int Main(int argc, char **argv) {
	try {
		auto app = lm::CreateApplication({ argc, argv });

		app->run();

	} catch (const std::exception &e) {
		std::println(stderr, "Citical error: {}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

} // namespace lm

#if defined(LUMEN_PLATFORM_WINDOWS) && defined(LUMEN_BUILD_WINDOWS_APP)

#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
	return lm::Main(__argc, __argv);
}

#else

int main(int argc, char **argv) {
	return lm::Main(argc, argv);
}

#endif
