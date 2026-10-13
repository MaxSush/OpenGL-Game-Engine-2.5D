#pragma once

#include<engine/engine_api.h>

#include <memory>
#include <spdlog/spdlog.h>

namespace Engine::Log
{
	ENGINE_API void Init();
	ENGINE_API void Shutdown();

	ENGINE_API std::shared_ptr<spdlog::logger>& GetEngineLogger();
	ENGINE_API std::shared_ptr<spdlog::logger>& GetGameLogger();
}

#if defined(ENGINE_BUILD)
	#define LOG_INTERNAL_GET_LOGGER() ::Engine::Log::GetEngineLogger()
#else
	#define LOG_INTERNAL_GET_LOGGER() ::Engine::Log::GetGameLogger()
#endif

#define LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(LOG_INTERNAL_GET_LOGGER(), __VA_ARGS__)
#define LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(LOG_INTERNAL_GET_LOGGER(), __VA_ARGS__)
#define LOG_INFO(...)     SPDLOG_LOGGER_INFO(LOG_INTERNAL_GET_LOGGER(), __VA_ARGS__)
#define LOG_WARN(...)     SPDLOG_LOGGER_WARN(LOG_INTERNAL_GET_LOGGER(), __VA_ARGS__)
#define LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(LOG_INTERNAL_GET_LOGGER(), __VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(LOG_INTERNAL_GET_LOGGER(), __VA_ARGS__)