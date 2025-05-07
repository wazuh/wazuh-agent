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
        std::optional<std::string> id;
        std::string condition;
        std::vector<std::unique_ptr<IRuleEvaluator>> rules;
    };

    /// @brief Constructor
    explicit SCAPolicy(std::string id, Check requirements, std::vector<Check> checks);

    SCAPolicy(SCAPolicy&& other) noexcept;

    /// @brief Runs the policy check
    /// @param scanInterval Scan interval in milliseconds
    /// @param scanOnStart Scan on start
    /// @param reportCheckResult Function to report check result
    /// @return Awaitable void
    boost::asio::awaitable<void>
    Run(std::time_t scanInterval,
        bool scanOnStart,
        std::function<void(const std::string&, const std::string&, const sca::CheckResult)> reportCheckResult);

    /// @brief Stops the policy check
    void Stop();

private:
    void
    Scan(const std::function<void(const std::string&, const std::string&, const sca::CheckResult)>& reportCheckResult);

    std::string m_id;
    Check m_requirements;
    std::vector<Check> m_checks;
    std::atomic<bool> m_keepRunning {true};
};
