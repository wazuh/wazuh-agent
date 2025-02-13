#include <logcollector.hpp>

#include <config.h>
#include <macos_reader.hpp>

#include <algorithm>
#include <cctype>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace
{

    std::vector<std::string> SplitAndTrim(const std::string& input, char delimiter = ',')
    {
        std::vector<std::string> result;
        std::stringstream ss(input);
        std::string item;

        auto isNotSpace = [](unsigned char ch)
        {
            return !std::isspace(ch);
        };

        while (std::getline(ss, item, delimiter))
        {
            item.erase(item.begin(), std::find_if(item.begin(), item.end(), isNotSpace));
            item.erase(std::find_if(item.rbegin(), item.rend(), isNotSpace).base(), item.end());
            if (!item.empty())
                result.push_back(item);
        }

        return result;
    }

} // namespace

namespace logcollector
{

    void Logcollector::AddPlatformSpecificReader(
        const std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
    {
        const auto fileWait = configurationParser->GetTimeConfigOrDefault(
            config::logcollector::DEFAULT_FILE_WAIT, "logcollector", "read_interval");

        const std::vector<std::map<std::string, std::string>> defaultMacOsConfig {};

        auto macosConfig = configurationParser->GetConfigOrDefault(defaultMacOsConfig, "logcollector", "macos");

        for (auto& entry : macosConfig)
        {
            const auto query = entry["query"];
            const auto level = entry["level"];
            const auto types = entry["type"];
            const auto typeList = SplitAndTrim(types);
            AddReader(std::make_shared<MacOSReader>(*this, fileWait, level, query, typeList));
        }
    }

} // namespace logcollector
