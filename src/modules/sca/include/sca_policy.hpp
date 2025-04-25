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
        std::string title;
        std::string condition;
        std::vector<std::unique_ptr<IRuleEvaluator>> rules;
    };

    /// @brief Constructor
    explicit SCAPolicy(Check requirements, std::vector<Check> checks);

    SCAPolicy(SCAPolicy&& other) noexcept;

    /// @brief Runs the policy check
    /// @return Awaitable void
    boost::asio::awaitable<void> Run();

    /// @brief Stops the policy check
    void Stop();

private:
    void ReportCheckResult(const Check& check, bool result);

    Check m_requirements;
    std::vector<Check> m_checks;
    std::atomic<bool> m_keepRunning {true};
    std::function<int(Message)> m_pushMessage;
};
