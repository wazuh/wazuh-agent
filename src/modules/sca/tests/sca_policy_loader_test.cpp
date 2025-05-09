#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <configuration_parser.hpp>
#include <sca_policy.hpp>
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

    const SCAPolicyLoader loader(fsMock, configurationParser, dbSync);
    ASSERT_EQ(loader.LoadPolicies([](auto, auto) { return; }).size(), 0);
}
