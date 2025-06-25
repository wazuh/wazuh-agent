#include <configuration_parser.hpp>

#include <config.h>
#include <yaml_utils.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <queue>
#include <unordered_set>
#include <utility>

namespace
{
    const std::filesystem::path CONFIG_FILE = std::filesystem::path(config::DEFAULT_CONFIG_PATH) / "wazuh-agent.yml";
}

namespace configuration
{
    ConfigurationParser::ConfigurationParser(std::filesystem::path configFilePath)
        : m_configFilePath(std::move(configFilePath))
    {
        LoadLocalConfig();
    }

    ConfigurationParser::ConfigurationParser()
        : ConfigurationParser(CONFIG_FILE)
    {
    }

    ConfigurationParser::ConfigurationParser(const std::string& stringToParse)
    {
        try
        {
            m_config = YAML::Load(stringToParse);
        }
        catch (const std::exception& e)
        {
            LogError("Error parsing yaml string: {}.", e.what());
            throw;
        }
    }

    void ConfigurationParser::LoadLocalConfig()
    {
        LogDebug("Loading local config file: {}.", m_configFilePath.string());

        try
        {
            if (!IsValidYamlFile(m_configFilePath))
            {
                throw std::runtime_error("The file does not contain a valid YAML structure.");
            }
            m_config = YAML::LoadFile(m_configFilePath.string());
        }
        catch (const std::exception& e)
        {
            m_config = YAML::Node();
            // LogWarn("Using default values due to error parsing wazuh-agent.yml file: {} - {}",
                    // e.what(),
                    // m_configFilePath.string());
        }
    }

    void ConfigurationParser::SetServerURL(const std::string& value)
    {
        try
        {
            m_config["agent"]["server_url"] = value;
            std::ofstream file(m_configFilePath);
            file << m_config;
            file.close();
        }
        catch (const std::exception& e)
        {
            // LogWarn("Error setting server URL: {}", e.what());
        }
    }

    bool ConfigurationParser::IsValidYamlFile(const std::filesystem::path& configFile) const
    {
        try
        {
            const YAML::Node mapToValidte = YAML::LoadFile(configFile.string());
            if (!mapToValidte.IsMap() && !mapToValidte.IsSequence())
            {
                throw std::runtime_error("The file does not contain a valid YAML structure.");
            }
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    void ConfigurationParser::LoadSharedConfig()
    {
        LogDebug("Loading shared configuration.");
        if (m_getGroups == nullptr)
        {
            // LogWarn("Load shared configuration failed, no get groups function set");
            return;
        }

        try
        {
            const std::vector<std::string> groupIds = m_getGroups();
            YAML::Node tmpConfig = m_config;

            for (const auto& groupId : groupIds)
            {
                const std::filesystem::path groupFile = std::filesystem::path(config::DEFAULT_SHARED_CONFIG_PATH) /
                                                        (groupId + config::DEFAULT_SHARED_FILE_EXTENSION);

                LogDebug("Loading group configuration file: {}.", groupFile.string());

                const YAML::Node fileToAppend = YAML::LoadFile(groupFile.string());

                if (!tmpConfig.IsDefined() || tmpConfig.IsNull())
                {
                    tmpConfig = fileToAppend;
                }
                else
                {
                    MergeYamlNodes(tmpConfig, fileToAppend);
                }
            }

            m_config = tmpConfig;
        }
        catch (const YAML::Exception& e)
        {
            // LogWarn("Load shared configuration failed: {}", e.what());
        }
    }

    void ConfigurationParser::SetGetGroupIdsFunction(std::function<std::vector<std::string>()> getGroupIdsFunction)
    {
        m_getGroups = std::move(getGroupIdsFunction);
        LoadSharedConfig();
    }

    void ConfigurationParser::ReloadConfiguration()
    {
        // LogInfo("Reload configuration.");

        // Reset saved configuration
        m_config = YAML::Node();

        // Load local configuration
        LoadLocalConfig();

        // Load shared configuration
        LoadSharedConfig();

        // LogInfo("Reload configuration done.");
    }
} // namespace configuration
