#pragma once

#include <configuration_parser.hpp>
#include <moduleWrapper.hpp>

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace centralized_configuration
{
    class CentralizedConfiguration
    {
    public:
        using SetGroupIdFunctionType = std::function<void(const std::vector<std::string>&)>;
        using DownloadGroupFilesFunctionType = std::function<bool(const std::string&, const std::string&)>;

        void Start() const;
        void Setup(const configuration::ConfigurationParser&) const;
        void Stop() const;
        Co_CommandExecutionResult ExecuteCommand(std::string command);
        std::string Name() const;

        void SetGroupIdFunction(SetGroupIdFunctionType setGroupIdFunction);
        void SetDownloadGroupFilesFunction(DownloadGroupFilesFunctionType downloadGroupFilesFunction);

    private:
        SetGroupIdFunctionType m_setGroupIdFunction;
        DownloadGroupFilesFunctionType m_downloadGroupFilesFunction;
    };
} // namespace centralized_configuration
