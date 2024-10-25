#include <logger.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>

Logger::Logger()
{
    auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("wazuh-agent", console_sink);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
}
