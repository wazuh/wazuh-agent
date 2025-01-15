#include "event_reader_win.hpp"

#include <config.h>
#include <timeHelper.h>

#include <chrono>
#include <map>

#include <logcollector.hpp>

namespace logcollector
{

void Logcollector::AddPlatformSpecificReader(std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    const auto refreshInterval = configurationParser->GetConfig<time_t>("logcollector", "channel_refresh").value_or(config::logcollector::CHANNEL_REFRESH_INTERVAL);

    const auto windowsConfig = configurationParser->GetConfig<std::vector<std::map<std::string, std::string>>>("logcollector", "windows").value_or(
        std::vector<std::map<std::string, std::string>> {});

    for (auto& entry : windowsConfig)
    {
        const auto channel = entry.at("channel");
        const auto query = entry.at("query");
        AddReader(std::make_shared<WindowsEventTracerReader>(*this, channel, query, refreshInterval));
    }
}

}
