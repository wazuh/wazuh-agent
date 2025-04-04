#pragma once

#include <isca_policy_loader.hpp>

#include <SCAPolicy.hpp>
#include <configuration_parser.hpp>
#include <ifilesystem_wrapper.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class SCAPolicyLoader : public ISCAPolicyLoader
{
public:
    using PolicyLoaderFunc = std::function<SCAPolicy(const std::filesystem::path&)>;

    SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                    std::shared_ptr<const configuration::ConfigurationParser> configurationParser = nullptr,
                    PolicyLoaderFunc loader = SCAPolicy::LoadFromFile);

    ~SCAPolicyLoader() = default;

    std::vector<SCAPolicy> GetPolicies() const;

private:
    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;
    PolicyLoaderFunc m_policyLoader;

    std::filesystem::path m_defaultPolicyPath;
    std::vector<std::filesystem::path> m_customPoliciesPaths;
    std::vector<std::filesystem::path> m_disabledPoliciesPaths;
};
