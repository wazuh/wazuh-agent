#include <sca_policy.hpp>

#include <check_condition_evaluator.hpp>
#include <sca_utils.hpp>

#include <logger.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

SCAPolicy::SCAPolicy(std::string id, Check requirements, std::vector<Check> checks)
    : m_id(std::move(id))
    , m_requirements(std::move(requirements))
    , m_checks(std::move(checks))
{
}

SCAPolicy::SCAPolicy(SCAPolicy&& other) noexcept
    : m_id(std::move(other.m_id))
    , m_requirements(std::move(other.m_requirements))
    , m_checks(std::move(other.m_checks))
    , m_keepRunning(other.m_keepRunning.load())
{
}

boost::asio::awaitable<void>
SCAPolicy::Run(std::time_t scanInterval,
               bool scanOnStart,
               std::function<void(const std::string&, const std::string&, const std::string&)> reportCheckResult,
               std::function<boost::asio::awaitable<void>(std::chrono::milliseconds)> wait)
{
    if (scanOnStart && m_keepRunning)
    {
        Scan(reportCheckResult);
    }

    while (m_keepRunning)
    {
        co_await wait(std::chrono::milliseconds(scanInterval));

        Scan(reportCheckResult);
    }
    co_return;
}

void SCAPolicy::Scan(
    const std::function<void(const std::string&, const std::string&, const std::string&)>& reportCheckResult)
{
    auto requirementsOk = sca::CheckResult::Passed;

    if (!m_requirements.rules.empty())
    {
        LogDebug("Starting Policy requirements evaluation for policy \"{}\".", m_id);

        auto resultEvaluator = CheckConditionEvaluator::FromString(m_requirements.condition);

        for (const auto& rule : m_requirements.rules)
        {
            if (!m_keepRunning)
            {
                return;
            }
            resultEvaluator.AddResult(rule->Evaluate());
        }

        requirementsOk = resultEvaluator.Result();

        LogDebug("Policy requirements evaluation completed for policy \"{}\", result: {}.",
                 m_id,
                 sca::CheckResultToString(requirementsOk));
    }

    if (requirementsOk == sca::CheckResult::Passed)
    {
        LogDebug("Starting Policy checks evaluation for policy \"{}\".", m_id);

        for (const auto& check : m_checks)
        {
            auto resultEvaluator = CheckConditionEvaluator::FromString(check.condition);

            for (const auto& rule : check.rules)
            {
                if (!m_keepRunning)
                {
                    return;
                }
                resultEvaluator.AddResult(rule->Evaluate());
            }

            const auto result = resultEvaluator.Result();

            // NOLINTBEGIN(bugprone-unchecked-optional-access)
            LogDebug("Policy check \"{}\" evaluation completed for policy \"{}\", result: {}.",
                     check.id.value(),
                     m_id,
                     sca::CheckResultToString(result));

            reportCheckResult(m_id, check.id.value(), sca::CheckResultToString(result));
            // NOLINTEND(bugprone-unchecked-optional-access)
        }

        LogDebug("Policy checks evaluation completed for policy \"{}\"", m_id);
    }
    else
    {
        for (const auto& check : m_checks)
        {
            // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
            reportCheckResult(m_id, check.id.value(), sca::CheckResultToString(sca::CheckResult::NotApplicable));
        }
    }
}

void SCAPolicy::Stop()
{
    m_keepRunning = false;
}
