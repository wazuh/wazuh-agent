#include <logger.hpp>

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

/// \cond UNIX

/// @brief Constructor for Logger.
Logger::Logger()
{
    auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>(LOGGER_NAME, console_sink);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::cfg::load_env_levels();
}

/// @brief Add Unix-specific sinks to the logger.
void Logger::AddPlatformSpecificSink() {}

/// \endcond
