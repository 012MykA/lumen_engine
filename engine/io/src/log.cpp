#include "lumen_engine/io/log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace lm {

void Log::Init() {
	spdlog::set_pattern("%^[%T] %n: %v%$");
	m_EngineLogger = spdlog::stdout_color_mt("Engine");
	m_EngineLogger->set_level(spdlog::level::trace);

	m_AppLogger = spdlog::stdout_color_mt("App");
	m_AppLogger->set_level(spdlog::level::trace);
}

} // namespace lm
