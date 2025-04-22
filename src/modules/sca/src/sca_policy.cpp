#include <sca_policy.hpp>

#include <check_condition_evaluator.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

SCAPolicy::SCAPolicy(std::vector<Check> checks)
    : m_checks(std::move(checks))
{
}

SCAPolicy::SCAPolicy(SCAPolicy&& other) noexcept
    : m_checks(std::move(other.m_checks))
    , m_keepRunning(other.m_keepRunning.load())
{
}

boost::asio::awaitable<void> SCAPolicy::Run()
{
    while (m_keepRunning)
    {
        auto requirementsOk = false;

        for (const auto& requirement : m_requirements)
        {
            auto resultEvaluator = CheckConditionEvaluator::fromString(requirement.condition);

            for (const auto& rule : requirement.rules)
            {
                resultEvaluator.addResult(rule->Evaluate() == RuleResult::Found);
            }

            requirementsOk = resultEvaluator.result();
        }

        if (requirementsOk)
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
        }

        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::steady_timer timer(executor);
        timer.expires_after(std::chrono::seconds(5)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        co_await timer.async_wait(boost::asio::use_awaitable);
    }
    co_return;
}

void SCAPolicy::Stop()
{
    m_keepRunning = false;
}

SCAPolicy SCAPolicy::LoadFromFile([[maybe_unused]] const std::filesystem::path& path)
{
    std::vector<Check> checks;
    return SCAPolicy(std::move(checks));
}
