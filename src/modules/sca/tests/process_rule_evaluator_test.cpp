#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sca_policy_check.hpp>

#include <mock_filesystem_wrapper.hpp>

#include <filesystem>
#include <memory>

class ProcessRuleEvaluatorTest : public ::testing::Test
{
protected:
    PolicyEvaluationContext m_ctx;
    std::unique_ptr<MockFileSystemWrapper> m_fsMock;
    MockFileSystemWrapper* m_rawFsMock = nullptr;
    std::function<std::vector<std::string>()> m_processesMock;

    void SetUp() override
    {
        m_fsMock = std::make_unique<MockFileSystemWrapper>();
        m_rawFsMock = m_fsMock.get();
    }

    ProcessRuleEvaluator CreateEvaluator()
    {
        return {m_ctx, std::move(m_fsMock), m_processesMock};
    }
};

TEST_F(ProcessRuleEvaluatorTest, ProcessFoundReturnsFound)
{
    m_ctx.rule = "myprocess";

    m_processesMock = []
    {
        return std::vector<std::string> {"init", "myprocess", "sshd"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(ProcessRuleEvaluatorTest, ProcessNotFoundReturnsInvalid)
{
    m_ctx.rule = "not-running";

    m_processesMock = []
    {
        return std::vector<std::string> {"systemd", "sshd", "nginx"};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Invalid);
}

TEST_F(ProcessRuleEvaluatorTest, EmptyProcessListReturnsInvalid)
{
    m_ctx.rule = "whatever";

    m_processesMock = []
    {
        return std::vector<std::string> {};
    };

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Invalid);
}
