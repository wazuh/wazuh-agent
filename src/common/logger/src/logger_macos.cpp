#include <logger.hpp>

// #include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

Logger::Logger()
{
    // auto sink = std::make_shared<spdlog::sinks::syslog_sink_mt>("wazuh-agent", LOG_PID, LOG_USER, true);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("wazuh-agent", console_sink);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
}
