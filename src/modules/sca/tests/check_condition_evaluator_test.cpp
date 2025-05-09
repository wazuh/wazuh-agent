#include <gtest/gtest.h>

#include <check_condition_evaluator.hpp>

TEST(CheckConditionEvaluatorTest, FromStringValidValues)
{
    EXPECT_NO_THROW({
        const auto evaluator = CheckConditionEvaluator::FromString("all");
        EXPECT_FALSE(evaluator.Result());
    });

    EXPECT_NO_THROW({
        const auto evaluator = CheckConditionEvaluator::FromString("any");
        EXPECT_FALSE(evaluator.Result());
    });

    EXPECT_NO_THROW({
        const auto evaluator = CheckConditionEvaluator::FromString("none");
        EXPECT_TRUE(evaluator.Result());
    });
}

TEST(CheckConditionEvaluatorTest, FromStringInvalidValueThrows)
{
    EXPECT_THROW(CheckConditionEvaluator::FromString("invalid"), std::invalid_argument);
}

TEST(CheckConditionEvaluatorTest, AllConditionBehavior)
{
    auto evaluator = CheckConditionEvaluator::FromString("all");

    // No rules added yet: should be false.
    EXPECT_FALSE(evaluator.Result());

    evaluator.AddResult(RuleResult::Found);
    evaluator.AddResult(RuleResult::Found);
    evaluator.AddResult(RuleResult::Found);
    EXPECT_TRUE(evaluator.Result());

    // Adding a failing rule
    auto evaluator2 = CheckConditionEvaluator::FromString("all");
    evaluator2.AddResult(RuleResult::Found);
    evaluator2.AddResult(RuleResult::NotFound);
    evaluator2.AddResult(RuleResult::Found);
    EXPECT_FALSE(evaluator2.Result());
}

TEST(CheckConditionEvaluatorTest, AnyConditionBehavior)
{
    auto evaluator = CheckConditionEvaluator::FromString("any");

    // No rules added yet: should be false.
    EXPECT_FALSE(evaluator.Result());

    evaluator.AddResult(RuleResult::NotFound);
    evaluator.AddResult(RuleResult::NotFound);
    EXPECT_FALSE(evaluator.Result());

    evaluator.AddResult(RuleResult::Found);
    EXPECT_TRUE(evaluator.Result());
}

TEST(CheckConditionEvaluatorTest, NoneConditionBehavior)
{
    auto evaluator = CheckConditionEvaluator::FromString("none");

    // No rules added yet: should be true.
    EXPECT_TRUE(evaluator.Result());

    evaluator.AddResult(RuleResult::NotFound);
    evaluator.AddResult(RuleResult::NotFound);
    EXPECT_TRUE(evaluator.Result());

    evaluator.AddResult(RuleResult::Found); // At least one passed -> should now be false.
    EXPECT_FALSE(evaluator.Result());
}

TEST(CheckConditionEvaluatorTest, AddResultStopsAfterResultDetermined)
{
    auto evaluator = CheckConditionEvaluator::FromString("any");

    evaluator.AddResult(RuleResult::Found); // Should determine result = true immediately
    EXPECT_TRUE(evaluator.Result());

    evaluator.AddResult(RuleResult::NotFound); // Should have no effect
    EXPECT_TRUE(evaluator.Result());

    auto evaluator2 = CheckConditionEvaluator::FromString("all");

    evaluator2.AddResult(RuleResult::NotFound); // Should determine result = false immediately
    EXPECT_FALSE(evaluator2.Result());

    evaluator2.AddResult(RuleResult::Found); // Should have no effect
    EXPECT_FALSE(evaluator2.Result());
}

TEST(CheckConditionEvaluatorTest, AllConditionWithInvalidMakesResultFalse)
{
    auto evaluator = CheckConditionEvaluator::FromString("all");

    evaluator.AddResult(RuleResult::Found);
    evaluator.AddResult(RuleResult::Found);
    evaluator.AddResult(RuleResult::Invalid);

    EXPECT_FALSE(evaluator.Result());
}

TEST(CheckConditionEvaluatorTest, NoneConditionWithInvalidMakesResultFalse)
{
    auto evaluator = CheckConditionEvaluator::FromString("none");

    evaluator.AddResult(RuleResult::NotFound);
    evaluator.AddResult(RuleResult::Invalid);

    EXPECT_FALSE(evaluator.Result());
}

TEST(CheckConditionEvaluatorTest, AnyConditionWithInvalidCanBeTrue)
{
    auto evaluator = CheckConditionEvaluator::FromString("any");

    evaluator.AddResult(RuleResult::NotFound);
    EXPECT_FALSE(evaluator.Result());

    evaluator.AddResult(RuleResult::Invalid);
    EXPECT_FALSE(evaluator.Result());

    evaluator.AddResult(RuleResult::Found);
    EXPECT_TRUE(evaluator.Result());
}
