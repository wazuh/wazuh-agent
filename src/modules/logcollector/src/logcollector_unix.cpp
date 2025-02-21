#include <config.h>
#include <journald_reader.hpp>
#include <logcollector.hpp>

#include <memory>

namespace logcollector
{

    void Logcollector::AddPlatformSpecificReader(
        const std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
    {
        const auto journaldConfigs = configurationParser->GetConfigOrDefault<YAML::Node>(
            YAML::Node(YAML::NodeType::Sequence), "logcollector", "journald");

        const auto fileWait = configurationParser->GetTimeConfigOrDefault(
            config::logcollector::DEFAULT_FILE_WAIT, "logcollector", "read_interval");

        for (const auto& config : journaldConfigs)
        {
            if (!config.IsMap())
                continue;

            if (config["conditions"])
            {
                // Handle multiple conditions case
                FilterGroup filters;
                for (const auto& condition : config["conditions"])
                {
                    filters.push_back({condition["field"].as<std::string>(),
                                       condition["value"].as<std::string>(),
                                       condition["exact_match"].as<bool>(true)});
                }

                if (!filters.empty())
                {
                    // Create a reader with all conditions
                    AddReader(std::make_shared<JournaldReader>(
                        *this, filters, config["ignore_if_missing"].as<bool>(false), fileWait));
                }
            }
            else
            {
                // Single condition case
                const FilterGroup filters {{config["field"].as<std::string>(),
                                            config["value"].as<std::string>(),
                                            config["exact_match"].as<bool>(true)}};

                AddReader(std::make_shared<JournaldReader>(
                    *this, filters, config["ignore_if_missing"].as<bool>(false), fileWait));
            }
        }
    }

} // namespace logcollector
