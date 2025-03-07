#include <logger.hpp>

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/win_eventlog_sink.h>

#include <memory>

/// \cond WINDOWS
/**
 * @brief Constructor for Logger.
 */
Logger::Logger()
{
    auto sink = std::make_shared<spdlog::sinks::win_eventlog_sink_mt>("Wazuh-Agent");
    auto logger = std::make_shared<spdlog::logger>(LOGGER_NAME, sink);

    spdlog::register_logger(logger);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::cfg::load_env_levels();
}

/**
 * @brief Add Windows-specific sinks to the logger.
 */
void Logger::AddPlatformSpecificSink()
{
    std::shared_ptr<spdlog::logger> logger = spdlog::get(LOGGER_NAME);

    if (logger)
    {
        auto stdOutSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        logger->sinks().clear();
        logger->sinks().push_back(stdOutSink);
    }
}

/// \endcond
