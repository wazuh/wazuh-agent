#include <gtest/gtest.h>

#include <sca_policy_check.hpp>

#include <filesystem>
#include <memory>

class RegistryRuleEvaluatorTest : public ::testing::Test
{
protected:
    PolicyEvaluationContext m_ctx;
    RegistryRuleEvaluator::IsValidRegistryKeyFunc m_isValidKey;
    RegistryRuleEvaluator::GetRegistryKeysFunc m_getKeys;
    RegistryRuleEvaluator::GetRegistryValuesFunc m_getValues;

    void SetUp() override
    {
        m_isValidKey = [](const std::string&)
        {
            return true;
        };
        m_getKeys = [](const std::string&, const std::string&)
        {
            return std::vector<std::string> {};
        };
        m_getValues = [](const std::string&, const std::string&)
        {
            return std::vector<std::string> {};
        };
    }

    RegistryRuleEvaluator CreateEvaluator()
    {
        return {m_ctx, m_isValidKey, m_getKeys, m_getValues};
    }
};

TEST_F(RegistryRuleEvaluatorTest, NoPatternValidKeyReturnsFound)
{
    m_ctx.pattern = std::nullopt;
    m_ctx.rule = "HKEY_LOCAL_MACHINE\\Software\\Something";
    m_isValidKey = [](const std::string&)
    {
        return true;
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(RegistryRuleEvaluatorTest, NoPatternInvalidKeyReturnsNotFound)
{
    m_ctx.pattern = std::nullopt;
    m_ctx.rule = "HKEY_LOCAL_MACHINE\\Software\\Missing";
    m_isValidKey = [](const std::string&)
    {
        return false;
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}

TEST_F(RegistryRuleEvaluatorTest, PatternArrowValueFoundReturnsFound)
{
    m_ctx.pattern = std::string("SubKey -> ExpectedValue");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&, const std::string&)
    {
        return std::vector<std::string> {"SomethingElse", "ExpectedValue"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(RegistryRuleEvaluatorTest, PatternArrowValueNotFoundReturnsNotFound)
{
    m_ctx.pattern = std::string("SubKey -> MissingValue");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&, const std::string&)
    {
        return std::vector<std::string> {"A", "B", "C"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}

TEST_F(RegistryRuleEvaluatorTest, PatternKeyNotFoundReturnsNotFound)
{
    m_ctx.pattern = std::string("TargetSubKey");
    m_ctx.rule = "HKEY_LOCAL_MACHINE\\Software";

    m_getKeys = [](const std::string&, const std::string&)
    {
        return std::vector<std::string> {"Unrelated", "SomethingElse"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}
