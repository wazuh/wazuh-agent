#include "event_reader_win.hpp"

#include <config.h>
#include <timeHelper.h>

#include <chrono>
#include <map>

#include "logcollector.hpp"

namespace logcollector
{

    void Logcollector::AddPlatformSpecificReader(
        std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
    {
        const auto refreshInterval = ParseTimeUnit(configurationParser->GetConfigOrDefault(
            config::logcollector::DEFAULT_CHANNEL_REFRESH_INTERVAL, "logcollector", "reload_interval"));

        const std::vector<std::map<std::string, std::string>> defaultWinOsConfig {};

        auto windowsConfig = configurationParser->GetConfigOrDefault(defaultWinOsConfig, "logcollector", "windows");

        for (auto& entry : windowsConfig)
        {
            const auto channel = entry["channel"];
            const auto query = entry["query"];
            AddReader(std::make_shared<winevt::WindowsEventTracerReader>(*this, channel, query, refreshInterval));
        }
    }

} // namespace logcollector
