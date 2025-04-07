#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <configuration_parser.hpp>
#include <isca_policy_loader.hpp>
#include <sca_policy_loader.hpp>

#include <mock_filesystem_wrapper.hpp>

#include <memory>

TEST(ScaPolicyLoaderTest, Contruction)
{
    auto fsMock = std::make_shared<testing::NiceMock<MockFileSystemWrapper>>();
    auto configurationParser = std::make_shared<configuration::ConfigurationParser>(std::string(R"()"));
    const SCAPolicyLoader loader(fsMock, configurationParser);
    SUCCEED();
}

TEST(ScaPolicyLoaderTest, NoPolicies)
{
    auto fsMock = std::make_shared<testing::NiceMock<MockFileSystemWrapper>>();
    auto configurationParser = std::make_shared<configuration::ConfigurationParser>(std::string(R"()"));
    const auto fakeLoader =
        [=](const std::filesystem::path&, std::function<int(Message)>) // NOLINT(performance-unnecessary-value-param)
    {
        return SCAPolicy {};
    };

    const std::filesystem::path defaultDir = "";
    EXPECT_CALL(*fsMock, exists(defaultDir)).WillOnce(testing::Return(false));

    const SCAPolicyLoader loader(fsMock, configurationParser, nullptr, fakeLoader);

    ASSERT_EQ(loader.GetPolicies().size(), 0);
}

TEST(ScaPolicyLoaderTest, GetPoliciesReturnsOnePolicyFromConfiguration)
{
    auto fsMock = std::make_shared<testing::NiceMock<MockFileSystemWrapper>>();

    auto configurationParser = std::make_shared<configuration::ConfigurationParser>(std::string(R"(
        sca:
          policies:
            - test_path
          policies_disabled:
            - test_path_disabled
    )"));

    const std::filesystem::path configPoliciesValue = "test_path";
    const std::filesystem::path configPoliciesDisabledValue = "test_path_disabled";
    const std::filesystem::path defaultDir = "";

    EXPECT_CALL(*fsMock, exists(configPoliciesValue)).WillOnce(testing::Return(true));
    EXPECT_CALL(*fsMock, exists(configPoliciesDisabledValue)).WillOnce(testing::Return(true));
    EXPECT_CALL(*fsMock, exists(defaultDir)).WillOnce(testing::Return(true));
    EXPECT_CALL(*fsMock, is_directory(defaultDir)).WillOnce(testing::Return(true));
    EXPECT_CALL(*fsMock, list_directory(defaultDir))
        .WillOnce(testing::Return(std::vector<std::filesystem::path> {configPoliciesDisabledValue}));

    EXPECT_CALL(*fsMock, is_regular_file(configPoliciesDisabledValue)).WillOnce(testing::Return(true));

    const auto fakeLoader =
        [=](const std::filesystem::path&, std::function<int(Message)>) // NOLINT(performance-unnecessary-value-param)
    {
        return SCAPolicy {};
    };
    const SCAPolicyLoader loader(fsMock, configurationParser, nullptr, fakeLoader);

    ASSERT_EQ(loader.GetPolicies().size(), 1);
}
