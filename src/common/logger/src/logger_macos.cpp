#include <logger.hpp>

#include <spdlog/sinks/syslog_sink.h>

Logger::Logger()
{
    auto sink = std::make_shared<spdlog::sinks::syslog_sink_mt>("wazuh-agent", LOG_PID, LOG_USER, true);
    auto logger = std::make_shared<spdlog::logger>("wazuh-agent", sink);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
}
