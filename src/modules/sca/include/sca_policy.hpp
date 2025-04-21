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
    std::vector<Check> m_requirements;
    std::vector<Check> m_checks;
    std::atomic<bool> m_keepRunning {true};
};
