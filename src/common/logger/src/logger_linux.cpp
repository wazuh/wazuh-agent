#include <logger.hpp>

#include <spdlog/sinks/systemd_sink.h>

Logger::Logger()
{
    auto sink = std::make_shared<spdlog::sinks::systemd_sink_st>();
    auto logger = std::make_shared<spdlog::logger>("wazuh-agent", sink);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
}
