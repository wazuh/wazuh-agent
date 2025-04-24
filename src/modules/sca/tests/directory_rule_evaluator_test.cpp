#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sca_policy_check.hpp>

#include <mock_file_io_utils.hpp>
#include <mock_filesystem_wrapper.hpp>

#include <filesystem>
#include <memory>

class DirRuleEvaluatorTest : public ::testing::Test
{
protected:
    PolicyEvaluationContext m_ctx;
    std::unique_ptr<MockFileSystemWrapper> m_fsMock;
    std::unique_ptr<MockFileIOUtils> m_ioMock;
    MockFileSystemWrapper* m_rawFsMock = nullptr;
    MockFileIOUtils* m_rawIoMock = nullptr;

    void SetUp() override
    {
        m_fsMock = std::make_unique<MockFileSystemWrapper>();
        m_rawFsMock = m_fsMock.get();
        m_ioMock = std::make_unique<MockFileIOUtils>();
        m_rawIoMock = m_ioMock.get();
    }

    DirRuleEvaluator CreateEvaluator()
    {
        return {m_ctx, std::move(m_fsMock), std::move(m_ioMock)};
    }
};

TEST_F(DirRuleEvaluatorTest, DirectoryDoesNotExistReturnsNotFound)
{
    m_ctx.pattern = std::nullopt;
    m_ctx.rule = "dir/";

    EXPECT_CALL(*m_rawFsMock, exists(std::filesystem::path("dir/"))).WillOnce(::testing::Return(false));

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}

TEST_F(DirRuleEvaluatorTest, ExistsButNotDirectoryReturnsNotFound)
{
    m_ctx.pattern = std::nullopt;
    m_ctx.rule = "dir/";

    EXPECT_CALL(*m_rawFsMock, exists(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, is_directory(std::filesystem::path("dir/"))).WillOnce(::testing::Return(false));

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}

TEST_F(DirRuleEvaluatorTest, NoPatternValidDirectoryReturnsFound)
{
    m_ctx.pattern = std::nullopt;
    m_ctx.rule = "dir/";

    EXPECT_CALL(*m_rawFsMock, exists(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, is_directory(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(DirRuleEvaluatorTest, RegexPatternMatchesFileReturnsFound)
{
    m_ctx.pattern = std::string("r:foo");
    m_ctx.rule = "dir/";

    EXPECT_CALL(*m_rawFsMock, exists(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, is_directory(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, list_directory(std::filesystem::path("dir/")))
        .WillOnce(::testing::Return(std::vector<std::filesystem::path> {"foo.txt", "bar.txt"}));

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(DirRuleEvaluatorTest, RegexPatternNoMatchReturnsNotFound)
{
    m_ctx.pattern = std::string("r:nomatch");
    m_ctx.rule = "dir/";

    EXPECT_CALL(*m_rawFsMock, exists(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, is_directory(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, list_directory(std::filesystem::path("dir/")))
        .WillOnce(::testing::Return(std::vector<std::filesystem::path> {"file1", "file2"}));

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::NotFound);
}

TEST_F(DirRuleEvaluatorTest, PatternWithArrowMatchesFileContentReturnsFound)
{
    m_ctx.pattern = std::string("target.txt -> hello");
    m_ctx.rule = "dir/";

    EXPECT_CALL(*m_rawFsMock, exists(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, is_directory(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, list_directory(std::filesystem::path("dir/")))
        .WillOnce(::testing::Return(std::vector<std::filesystem::path> {"target.txt"}));
    EXPECT_CALL(*m_rawIoMock, readLineByLine(std::filesystem::path("dir/"), ::testing::_))
        .WillOnce(
            [](const auto&, const auto& callback)
            {
                callback("not this");
                callback("hello"); // triggers return false
            });

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}

TEST_F(DirRuleEvaluatorTest, ExactPatternMatchesFileNameReturnsFound)
{
    m_ctx.pattern = std::string("match.txt");
    m_ctx.rule = "dir/";

    EXPECT_CALL(*m_rawFsMock, exists(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, is_directory(std::filesystem::path("dir/"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_rawFsMock, list_directory(std::filesystem::path("dir/")))
        .WillOnce(::testing::Return(std::vector<std::filesystem::path> {"foo", "match.txt"}));

    auto evaluator = CreateEvaluator();
    EXPECT_EQ(evaluator.Evaluate(), RuleResult::Found);
}
