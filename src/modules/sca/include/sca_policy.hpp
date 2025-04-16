#pragma once

#include <message.hpp>
#include <sca_policy_check.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <atomic>
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

    SCAPolicy(SCAPolicy&& other) noexcept
        : m_checks(std::move(other.m_checks))
        , m_keepRunning(other.m_keepRunning.load())
    {
    }

    /// @brief Runs the policy check
    /// @return Awaitable void
    boost::asio::awaitable<void> Run()
    {
        while (m_keepRunning)
        {
            for (const auto& check : m_checks)
            {
                auto resultEvaluator = CheckConditionEvaluator::fromString(check.condition);

                for (const auto& rule : check.rules)
                {
                    resultEvaluator.addResult(rule->Evaluate() == RuleResult::Found);
                }

                [[maybe_unused]] auto result = resultEvaluator.result();

                ReportCheckResult(check, result);
            }

            auto executor = co_await boost::asio::this_coro::executor;
            boost::asio::steady_timer timer(executor);
            timer.expires_after(std::chrono::seconds(5));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }
        co_return;
    }

    void Stop()
    {
        m_keepRunning = false;
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
    void ReportCheckResult([[maybe_unused]] const Check& check, [[maybe_unused]] bool result)
    {
        // Send the check result to the pushMessage function
        if (m_pushMessage)
        {
            // TODO
        }
    }

    std::vector<Check> m_checks;
    std::atomic<bool> m_keepRunning {true};
    std::function<int(Message)> m_pushMessage;
};
