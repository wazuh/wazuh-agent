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
    struct Check
    {
        std::string id;
        std::string title;
        std::string condition;
        std::vector<std::unique_ptr<IRuleEvaluator>> rules;
    };

    /// @brief Constructor
    SCAPolicy(std::vector<Check> checks)
        : m_checks(std::move(checks))
    {
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
        std::vector<Check> checks;
        return SCAPolicy(std::move(checks));
    }

private:
    std::vector<Check> m_checks;
};
