#pragma once

#include <isca_policy_loader.hpp>

#include <SCAPolicy.hpp>
#include <configuration_parser.hpp>
#include <ifilesystem_wrapper.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class SCAPolicyLoader : public ISCAPolicyLoader
{
public:
    SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                    std::shared_ptr<configuration::ConfigurationParser> configurationParser = nullptr);

    ~SCAPolicyLoader() = default;

    std::vector<SCAPolicy> GetPolicies() const;

private:
    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;

    std::filesystem::path m_defaultPolicyPath;
    std::vector<std::filesystem::path> m_customPoliciesPaths;
    std::vector<std::filesystem::path> m_disabledPoliciesPaths;
};
