#include <sca_policy.hpp>

#include <check_condition_evaluator.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

SCAPolicy::SCAPolicy(std::vector<Check> checks)
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
            auto resultEvaluator = CheckConditionEvaluator::FromString(requirement.condition);

            for (const auto& rule : requirement.rules)
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

                [[maybe_unused]] auto result = resultEvaluator.Result();

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

SCAPolicy SCAPolicy::LoadFromFile([[maybe_unused]] const std::filesystem::path& path)
{
    std::vector<Check> checks;
    return SCAPolicy(std::move(checks));
}

void SCAPolicy::ReportCheckResult([[maybe_unused]] const Check& check, [[maybe_unused]] bool result)
{
    // Send the check result to the pushMessage function
    if (m_pushMessage)
    {
        // TODO
    }
}
