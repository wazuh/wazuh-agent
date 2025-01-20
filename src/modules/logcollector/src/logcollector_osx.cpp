#include <logcollector.hpp>

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
        const auto fileWait = configurationParser->GetConfig<time_t>("logcollector", "file_wait")
                                  .value_or(config::logcollector::DEFAULT_FILE_WAIT);

        auto macosConfig =
            configurationParser->GetConfig<std::vector<std::map<std::string, std::string>>>("logcollector", "macos")
                .value_or(std::vector<std::map<std::string, std::string>> {});

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
