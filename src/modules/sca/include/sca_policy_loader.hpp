#pragma once

#include <isca_policy_loader.hpp>

#include <configuration_parser.hpp>
#include <ifilesystem_wrapper.hpp>
#include <message.hpp>
#include <sca_policy.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class SCAPolicyLoader : public ISCAPolicyLoader
{
public:
    /// @brief Type alias for a function that loads a policy from a SCA Policy yaml file
    using PolicyLoaderFunc = std::function<SCAPolicy(const std::filesystem::path&, std::function<int(Message)>)>;

    /// @brief Constructor for SCAPolicyLoader
    /// @param fileSystemWrapper A shared pointer to a file system wrapper
    /// @param configurationParser A shared pointer to a configuration parser
    /// @param pushMessage A function that pushes messages
    /// @param loader A function that loads a policy from a SCA Policy yaml file
    SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                    std::shared_ptr<const configuration::ConfigurationParser> configurationParser = nullptr,
                    std::function<int(Message)> pushMessage = nullptr,
                    PolicyLoaderFunc loader = SCAPolicy::LoadFromFile);

    /// @brief Destructor for SCAPolicyLoader
    ~SCAPolicyLoader() = default;

    /// @brief Loads SCA Policies
    /// @returns a vector of SCAPolicy objects
    std::vector<SCAPolicy> GetPolicies() const;

private:
    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;
    std::function<int(Message)> m_pushMessage;
    PolicyLoaderFunc m_policyLoader;

    std::vector<std::filesystem::path> m_customPoliciesPaths;
    std::vector<std::filesystem::path> m_disabledPoliciesPaths;
};
