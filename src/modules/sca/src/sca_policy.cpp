#include <sca_policy.hpp>

#include <check_condition_evaluator.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

SCAPolicy::SCAPolicy(Check requirements, std::vector<Check> checks)
    : m_requirements(std::move(requirements))
    , m_checks(std::move(checks))
{
}

SCAPolicy::SCAPolicy(SCAPolicy&& other) noexcept
    : m_requirements(std::move(other.m_requirements))
    , m_checks(std::move(other.m_checks))
    , m_keepRunning(other.m_keepRunning.load())
{
}

boost::asio::awaitable<void> SCAPolicy::Run()
{
    while (m_keepRunning)
    {
        auto requirementsOk = true;

        if (!m_requirements.rules.empty())
        {
            auto resultEvaluator = CheckConditionEvaluator::FromString(m_requirements.condition);

            for (const auto& rule : m_requirements.rules)
            {
                resultEvaluator.AddResult(rule->Evaluate() == RuleResult::Found);
            }

            requirementsOk = resultEvaluator.Result();
        }

        if (requirementsOk)
        {
            for (const auto& check : m_checks)
            {
                auto resultEvaluator = CheckConditionEvaluator::FromString(check.condition);

                for (const auto& rule : check.rules)
                {
                    resultEvaluator.AddResult(rule->Evaluate() == RuleResult::Found);
                }

                const auto result = resultEvaluator.Result();

                ReportCheckResult(check, result);
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

void SCAPolicy::ReportCheckResult([[maybe_unused]] const Check& check, [[maybe_unused]] bool result)
{
    // Send the check result to the pushMessage function
    if (m_pushMessage)
    {
        // TODO
    }
}
