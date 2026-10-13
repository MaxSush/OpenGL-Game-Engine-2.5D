#include <engine/core/logger/log.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Engine::Log
{
	namespace
	{
		std::shared_ptr<spdlog::logger> s_EngineLogger;
		std::shared_ptr<spdlog::logger> s_GameLogger;
	}

	std::shared_ptr<spdlog::logger>& GetEngineLogger()
	{
		return s_EngineLogger;
	}
	
	std::shared_ptr<spdlog::logger>& GetGameLogger()
	{
		return s_GameLogger;
	}

	void Engine::Log::Init()
	{
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_pattern("[%T.%e] [%^%l%$] [%n] [%s:%#] %v");

		auto engineFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/engine.log", true);

		auto gameFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/game.log", true);

		s_EngineLogger = std::make_shared<spdlog::logger>("Engine", spdlog::sinks_init_list{ consoleSink, engineFileSink });

		s_GameLogger = std::make_shared<spdlog::logger>("Game", spdlog::sinks_init_list{ consoleSink, gameFileSink });

		s_EngineLogger->set_level(spdlog::level::trace);
		s_GameLogger->set_level(spdlog::level::trace);

		s_EngineLogger->flush_on(spdlog::level::warn);
		s_GameLogger->flush_on(spdlog::level::warn);

	}

	void Shutdown()
	{
		s_GameLogger.reset();
		s_EngineLogger.reset();
	}
}