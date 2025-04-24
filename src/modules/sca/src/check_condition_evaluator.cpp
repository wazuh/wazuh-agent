#include <check_condition_evaluator.hpp>

CheckConditionEvaluator CheckConditionEvaluator::fromString(const std::string& str)
{
    if (str == "all")
    {
        return CheckConditionEvaluator {ConditionType::All};
    }
    if (str == "any")
    {
        return CheckConditionEvaluator {ConditionType::Any};
    }
    if (str == "none")
    {
        return CheckConditionEvaluator {ConditionType::None};
    }
    throw std::invalid_argument("Invalid condition type: " + str);
}

CheckConditionEvaluator::CheckConditionEvaluator(ConditionType type)
    : m_type {type}
{
}

void CheckConditionEvaluator::addResult(bool passed)
{
    if (m_result.has_value())
    {
        return;
    }

    ++m_totalRules;
    m_passedRules += passed;

    switch (m_type)
    {
        case ConditionType::All:
            if (!passed)
            {
                m_result = false;
            }
            break;
        case ConditionType::Any:
            if (passed)
            {
                m_result = true;
            }
            break;
        case ConditionType::None:
            if (passed)
            {
                m_result = false;
            }
            break;
    }
}

bool CheckConditionEvaluator::result() const
{
    if (m_result.has_value())
    {
        return *m_result;
    }

    switch (m_type)
    {
        case ConditionType::All: return m_totalRules > 0 && m_passedRules == m_totalRules;
        case ConditionType::Any: return m_passedRules > 0;
        case ConditionType::None: return m_passedRules == 0;
    }

    return false;
}
