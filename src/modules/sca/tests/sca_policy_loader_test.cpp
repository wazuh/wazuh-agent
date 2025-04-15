#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <configuration_parser.hpp>
#include <isca_policy_loader.hpp>
#include <sca_policy_loader.hpp>

#include "mocks/mockdbsync.hpp"
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
    auto dbSync = std::make_shared<MockDBSync>();
    const auto fakeLoader = [=](const std::filesystem::path&)
    {
        std::vector<SCAPolicy::Check> checks;
        return SCAPolicy {std::move(checks)};
    };

    const SCAPolicyLoader loader(fsMock, configurationParser, dbSync, fakeLoader);
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
    auto dbSync = std::make_shared<MockDBSync>();

    const std::filesystem::path configPoliciesValue = "test_path";
    const std::filesystem::path configPoliciesDisabledValue = "test_path_disabled";

    EXPECT_CALL(*fsMock, exists(configPoliciesValue)).WillOnce(testing::Return(true));
    EXPECT_CALL(*fsMock, exists(configPoliciesDisabledValue)).WillOnce(testing::Return(true));

    const auto fakeLoader = [=](const std::filesystem::path&)
    {
        std::vector<SCAPolicy::Check> checks;
        return SCAPolicy {std::move(checks)};
    };
    const SCAPolicyLoader loader(fsMock, configurationParser, dbSync, fakeLoader);

    ASSERT_EQ(loader.GetPolicies().size(), 1);
}
