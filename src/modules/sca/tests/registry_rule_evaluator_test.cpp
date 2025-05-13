#include <gtest/gtest.h>

#include <sca_policy_check.hpp>

#include <filesystem>
#include <memory>

class RegistryRuleEvaluatorTest : public ::testing::Test
{
protected:
    PolicyEvaluationContext m_ctx;
    RegistryRuleEvaluator::IsValidRegistryKeyFunc m_isValidKey;
    RegistryRuleEvaluator::GetRegistryValuesFunc m_getValues;
    RegistryRuleEvaluator::GetRegistryKeyValueFunc m_getKeyValue;

    void SetUp() override
    {
        m_isValidKey = [](const std::string&)
        {
            return true;
        };
        m_getValues = [](const std::string&)
        {
            return std::vector<std::string> {};
        };
        m_getKeyValue = [](const std::string&, const std::string&)
        {
            return std::string {};
        };
    }

    RegistryRuleEvaluator CreateEvaluator()
    {
        return {m_ctx, m_isValidKey, m_getValues, m_getKeyValue};
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

TEST_F(RegistryRuleEvaluatorTest, PatternKeyFoundReturnsFound)
{
    m_ctx.pattern = std::string("ExpectedKey");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&)
    {
        return std::vector<std::string> {"SomethingElse", "ExpectedKey"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(RegistryRuleEvaluatorTest, PatternKeyNotFoundReturnsNotFound)
{
    m_ctx.pattern = std::string("MissingKey");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&)
    {
        return std::vector<std::string> {"SomethingElse", "ExpectedKey"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}

TEST_F(RegistryRuleEvaluatorTest, PatternRegexKeyFoundReturnsFound)
{
    m_ctx.pattern = std::string("r:ExpectedKey");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&)
    {
        return std::vector<std::string> {"SomethingElse", "ExpectedKey"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(RegistryRuleEvaluatorTest, PatternRegexKeyNotFoundReturnsNotFound)
{
    m_ctx.pattern = std::string("r:MissingKey");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&)
    {
        return std::vector<std::string> {"SomethingElse", "ExpectedKey"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}

TEST_F(RegistryRuleEvaluatorTest, PatternArrowValueFoundReturnsFound)
{
    m_ctx.pattern = std::string("ExpectedKey -> ExpectedValue");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&)
    {
        return std::vector<std::string> {"SomethingElse", "ExpectedKey"};
    };

    m_getKeyValue = [](const std::string&, const std::string&)
    {
        return std::string {"ExpectedValue"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(RegistryRuleEvaluatorTest, PatternArrowValueNotFoundReturnsNotFound)
{
    m_ctx.pattern = std::string("ExpectedKey -> MissingValue");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&)
    {
        return std::vector<std::string> {"SomethingElse", "ExpectedKey"};
    };

    m_getKeyValue = [](const std::string&, const std::string&)
    {
        return std::string {"ExpectedValue"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}

TEST_F(RegistryRuleEvaluatorTest, PatternArrowRegexValueFoundReturnsFound)
{
    m_ctx.pattern = std::string("ExpectedKey -> r:ExpectedValue");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&)
    {
        return std::vector<std::string> {"SomethingElse", "ExpectedKey"};
    };

    m_getKeyValue = [](const std::string&, const std::string&)
    {
        return std::string {"ExpectedValue"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(RegistryRuleEvaluatorTest, PatternArrowRegexValueNotFoundReturnsNotFound)
{
    m_ctx.pattern = std::string("ExpectedKey -> r:MissingValue");
    m_ctx.rule = "HKEY_CURRENT_USER\\MyApp";

    m_getValues = [](const std::string&)
    {
        return std::vector<std::string> {"SomethingElse", "ExpectedKey"};
    };

    m_getKeyValue = [](const std::string&, const std::string&)
    {
        return std::string {"ExpectedValue"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}
