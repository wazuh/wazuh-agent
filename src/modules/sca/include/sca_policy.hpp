#pragma once

#include <message.hpp>
#include <sca_policy_check.hpp>

#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
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
    explicit SCAPolicy(std::vector<Check> checks);

    SCAPolicy(SCAPolicy&& other) noexcept;

    /// @brief Runs the policy check
    /// @return Awaitable void
    boost::asio::awaitable<void> Run();

    /// @brief Stops the policy check
    void Stop();

    /// @brief Loads a policy from a SCA Policy yaml file
    /// @param path The path to the SCA Policy yaml file
    /// @returns A SCAPolicy object
    /// @note This function is a placeholder and should be implemented in the actual code
    static SCAPolicy LoadFromFile([[maybe_unused]] const std::filesystem::path& path);

private:
    void ReportCheckResult([[maybe_unused]] const Check& check, [[maybe_unused]] bool result);

    std::vector<Check> m_requirements;
    std::vector<Check> m_checks;
    std::atomic<bool> m_keepRunning {true};
    std::function<int(Message)> m_pushMessage;
};
