#include <sca_policy_loader.hpp>

#include <filesystem_wrapper.hpp>

SCAPolicyLoader::SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper,
                                 std::shared_ptr<configuration::ConfigurationParser> configurationParser)
    : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                            : std::make_shared<file_system::FileSystemWrapper>())
{
    m_customPoliciesPaths = [this, &configurationParser]()
    {
        std::vector<std::filesystem::path> policiesPaths;
        std::vector<std::string> policies;
        policies = configurationParser->GetConfigOrDefault(policies, "sca", "policies");
        for (const auto& policy : policies)
        {
            if (m_fileSystemWrapper->exists(policy))
            {
                policiesPaths.push_back(policy);
            }
            else
            {
                LogWarn("Policy file does not exist: {}", policy);
            }
        }
        return policiesPaths;
    }();
    m_disabledPoliciesPaths = [this, &configurationParser]()
    {
        std::vector<std::filesystem::path> policiesPaths;
        std::vector<std::string> policies;
        policies = configurationParser->GetConfigOrDefault(policies, "sca", "policies_disabled");
        for (const auto& policy : policies)
        {
            if (m_fileSystemWrapper->exists(policy))
            {
                policiesPaths.push_back(policy);
            }
            else
            {
                LogWarn("Policy file does not exist: {}", policy);
            }
        }
        return policiesPaths;
    }();
}
