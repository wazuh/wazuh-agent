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

    evaluator.AddResult(true);
    evaluator.AddResult(true);
    evaluator.AddResult(true);
    EXPECT_TRUE(evaluator.Result());

    // Adding a failing rule
    auto evaluator2 = CheckConditionEvaluator::FromString("all");
    evaluator2.AddResult(true);
    evaluator2.AddResult(false);
    evaluator2.AddResult(true);
    EXPECT_FALSE(evaluator2.Result());
}

TEST(CheckConditionEvaluatorTest, AnyConditionBehavior)
{
    auto evaluator = CheckConditionEvaluator::FromString("any");

    // No rules added yet: should be false.
    EXPECT_FALSE(evaluator.Result());

    evaluator.AddResult(false);
    evaluator.AddResult(false);
    EXPECT_FALSE(evaluator.Result());

    evaluator.AddResult(true);
    EXPECT_TRUE(evaluator.Result());
}

TEST(CheckConditionEvaluatorTest, NoneConditionBehavior)
{
    auto evaluator = CheckConditionEvaluator::FromString("none");

    // No rules added yet: should be true.
    EXPECT_TRUE(evaluator.Result());

    evaluator.AddResult(false);
    evaluator.AddResult(false);
    EXPECT_TRUE(evaluator.Result());

    evaluator.AddResult(true); // At least one passed -> should now be false.
    EXPECT_FALSE(evaluator.Result());
}

TEST(CheckConditionEvaluatorTest, AddResultStopsAfterResultDetermined)
{
    auto evaluator = CheckConditionEvaluator::FromString("any");

    evaluator.AddResult(true); // Should determine result = true immediately
    EXPECT_TRUE(evaluator.Result());

    evaluator.AddResult(false); // Should have no effect
    EXPECT_TRUE(evaluator.Result());

    auto evaluator2 = CheckConditionEvaluator::FromString("all");

    evaluator2.AddResult(false); // Should determine result = false immediately
    EXPECT_FALSE(evaluator2.Result());

    evaluator2.AddResult(true); // Should have no effect
    EXPECT_FALSE(evaluator2.Result());
}
