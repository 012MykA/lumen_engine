#pragma once

#include "lumen_engine/core/export.hpp"

#include <spdlog/fmt/std.h>
#include <spdlog/spdlog.h>

#include <memory>

namespace lm {

class LUMEN_API Log {
public:
	static void Init();

	static std::shared_ptr<spdlog::logger> &GetEngineLogger() { return m_EngineLogger; }
	static std::shared_ptr<spdlog::logger> &GetAppLogger() { return m_AppLogger; }

private:
	inline static std::shared_ptr<spdlog::logger> m_EngineLogger;
	inline static std::shared_ptr<spdlog::logger> m_AppLogger;
};

} // namespace lm

#ifndef LUMEN_DISABLE_LOGGING

// clang-format off
	// Core log macros
    #define LUMEN_TRACE(...)			::lm::Log::GetEngineLogger()->trace(__VA_ARGS__)
    #define LUMEN_INFO(...)				::lm::Log::GetEngineLogger()->info(__VA_ARGS__)
    #define LUMEN_WARN(...)				::lm::Log::GetEngineLogger()->warn(__VA_ARGS__)
    #define LUMEN_ERROR(...)			::lm::Log::GetEngineLogger()->error(__VA_ARGS__)
    #define LUMEN_CRITICAL(...)			::lm::Log::GetEngineLogger()->critical(__VA_ARGS__)

    // Client log macros
    #define APP_TRACE(...)              ::lm::Log::GetAppLogger()->trace(__VA_ARGS__)
    #define APP_INFO(...)               ::lm::Log::GetAppLogger()->info(__VA_ARGS__)
    #define APP_WARN(...)               ::lm::Log::GetAppLogger()->warn(__VA_ARGS__)
    #define APP_ERROR(...)              ::lm::Log::GetAppLogger()->error(__VA_ARGS__)
    #define APP_CRITICAL(...)           ::lm::Log::GetAppLogger()->critical(__VA_ARGS__)
// clang-format on

#else

	// Core log macros
	#define LUMEN_CORE_TRACE(...)
	#define LUMEN_CORE_INFO(...)
	#define LUMEN_CORE_WARN(...)
	#define LUMEN_CORE_ERROR(...)
	#define LUMEN_CORE_CRITICAL(...)

	// Client log macros
	#define APP_TRACE(...)
	#define APP_INFO(...)
	#define APP_WARN(...)
	#define APP_ERROR(...)
	#define APP_CRITICAL(...)

#endif
