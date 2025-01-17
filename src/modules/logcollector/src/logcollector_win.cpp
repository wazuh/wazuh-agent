#include "event_reader_win.hpp"

#include <config.h>
#include <timeHelper.h>

#include <chrono>
#include <map>

#include "logcollector.hpp"

namespace logcollector
{

void Logcollector::AddPlatformSpecificReader(std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    const auto refreshInterval = configurationParser->GetConfig<time_t>("logcollector", "channel_refresh").value_or(config::logcollector::DEFAULT_CHANNEL_REFRESH_INTERVAL);

    auto windowsConfig = configurationParser->GetConfig<std::vector<std::map<std::string, std::string>>>("logcollector", "windows").value_or(
        std::vector<std::map<std::string, std::string>> {});

    for (auto& entry : windowsConfig)
    {
        const auto channel = entry["channel"];
        const auto query = entry["query"];
        AddReader(std::make_shared<winevt::WindowsEventTracerReader>(*this, channel, query, refreshInterval));
    }
}

}
