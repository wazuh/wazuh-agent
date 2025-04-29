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

    const std::time_t scanInterval {3600};

    const SCAPolicyLoader loader(fsMock, configurationParser, dbSync);
    ASSERT_EQ(loader.GetPolicies([](auto, auto) { return; }, scanInterval, true).size(), 0);
}
