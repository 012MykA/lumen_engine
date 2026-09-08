#include "lumen_engine/io/log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace lm {

namespace {

std::shared_ptr<spdlog::logger> engineLogger;
std::shared_ptr<spdlog::logger> appLogger;

} // namespace

void Log::Init() {
	spdlog::set_pattern("%^[%T] %n: %v%$");
	engineLogger = spdlog::stdout_color_mt("Engine");
	engineLogger->set_level(spdlog::level::trace);

	appLogger = spdlog::stdout_color_mt("App");
	appLogger->set_level(spdlog::level::trace);
}

std::shared_ptr<spdlog::logger> &Log::GetEngineLogger() {
	return engineLogger;
}

std::shared_ptr<spdlog::logger> &Log::GetAppLogger() {
	return appLogger;
}

} // namespace lm
