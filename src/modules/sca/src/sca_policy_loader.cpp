#include <sca_policy_loader.hpp>

#include <filesystem_wrapper.hpp>

SCAPolicyLoader::SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper,
                                 std::shared_ptr<configuration::ConfigurationParser> configurationParser)
    : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                            : std::make_shared<file_system::FileSystemWrapper>())
{
    const auto loadPoliciesPathsFromConfig = [this, &configurationParser](const std::string& configKey)
    {
        std::vector<std::filesystem::path> policiesPaths;
        std::vector<std::string> policies;
        policies = configurationParser->GetConfigOrDefault(policies, "sca", configKey);
        for (const auto& policy : policies)
        {
            if (m_fileSystemWrapper->exists(policy))
            {
                policiesPaths.emplace_back(policy);
            }
            else
            {
                LogWarn("Policy file does not exist: {}", policy);
            }
        }
        return policiesPaths;
    };

    m_customPoliciesPaths = loadPoliciesPathsFromConfig("policies");
    m_disabledPoliciesPaths = loadPoliciesPathsFromConfig("policies_disabled");
}
