#pragma once

#include <isca_policy_loader.hpp>

#include <configuration_parser.hpp>
#include <ifilesystem_wrapper.hpp>

#include <filesystem>
#include <memory>
#include <vector>

class SCAPolicyLoader : public ISCAPolicyLoader
{
public:
    SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                    std::shared_ptr<configuration::ConfigurationParser> configurationParser = nullptr);

    ~SCAPolicyLoader() = default;

private:
    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;
    std::vector<std::filesystem::path> m_customPoliciesPaths;
    std::vector<std::filesystem::path> m_disabledPoliciesPaths;
};
