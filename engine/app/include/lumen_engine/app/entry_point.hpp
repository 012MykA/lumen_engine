#pragma once

#include "lumen_engine/app/application.hpp"
#include "lumen_engine/logging/log.hpp"

#include <stdexcept>

namespace lm {

inline int Main(int argc, char **argv) {
	Log::Init();

	try {
		auto app = lm::CreateApplication({ argc, argv });
		app->Run();

	} catch (const std::exception &e) {
		LUMEN_CORE_CRITICAL("Citical: {}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

} // namespace lm

#if defined(LUMEN_PLATFORM_WINDOWS) && defined(LUMEN_BUILD_WINDOWS_APP)

	#include <Windows.h>

int WINMAIN WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
	return lm::Main(__argc, __argv);
}

#else

int main(int argc, char **argv) {
	return lm::Main(argc, argv);
}

#endif
