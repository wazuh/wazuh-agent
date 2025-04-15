#pragma once

#include <message.hpp>
#include <sca_policy_check.hpp>

#include <boost/asio/awaitable.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <vector>

class SCAPolicy
{
public:
    /// @brief Constructor
    SCAPolicy()
    {
        // instantiate checks
        // check id
        // check title
        // check condition
        // set condition (all, any, none, any required (required deprecated), all required (required deprecated))

        // now RULES
        // regex engine type
        // is it negated?
        // possible types: WM_SCA_TYPE_FILE, WM_SCA_TYPE_REGISTRY, WM_SCA_TYPE_PROCESS, WM_SCA_TYPE_DIR,
        // WM_SCA_TYPE_COMMAND.
        // do sca check
        // check results
        // report
    }

    /// @brief Runs the policy check
    /// @return Awaitable void
    boost::asio::awaitable<void> Run()
    {
        co_return;
    }

    /// @brief Loads a policy from a SCA Policy yaml file
    /// @param path The path to the SCA Policy yaml file
    /// @returns A SCAPolicy object
    /// @note This function is a placeholder and should be implemented in the actual code
    static SCAPolicy LoadFromFile([[maybe_unused]] const std::filesystem::path& path)
    {
        return {};
    }

private:
    std::vector<std::pair<int, std::vector<std::unique_ptr<IRuleEvaluator>>>> m_checks;
};
