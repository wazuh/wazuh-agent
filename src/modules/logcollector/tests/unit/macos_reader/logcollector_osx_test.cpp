#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <configuration_parser.hpp>
#include <logcollector.hpp>
#include <logcollector_mock.hpp>
#include <reader.hpp>

#include <memory>
#include <string>

using namespace logcollector;

class LogcollectorTest : public ::testing::TestWithParam<std::string> {};

TEST_P(LogcollectorTest, SetupHandlesMacOSConfig) {
    auto config_raw = GetParam();

    std::shared_ptr<IReader> capturedReader1;
    std::shared_ptr<IReader> capturedReader2;
    auto logcollector = LogcollectorMock();
    auto config = std::make_shared<configuration::ConfigurationParser>(config_raw);

    EXPECT_CALL(logcollector, AddReader(::testing::_)).Times(2)
        .WillOnce(::testing::SaveArg<0>(&capturedReader1))
        .WillOnce(::testing::SaveArg<0>(&capturedReader2));

    logcollector.AddPlatformSpecificReader(config);

    ASSERT_NE(capturedReader1, nullptr);
    ASSERT_NE(capturedReader2, nullptr);
}

INSTANTIATE_TEST_SUITE_P(
    Configurations,
    LogcollectorTest,
    ::testing::Values(
        // All fields are present
        R"(
        logcollector:
          enabled: true
          reload_interval: 60
          file_wait: 500
          macos:
            - query: process == "sshd" OR message CONTAINS "invalid"
              level: info
              type: trace,activity,log
            - query: process == "wazuh-agent"
              level: debug
              type: log
        )",
        // Missing level type and level
        R"(
        logcollector:
          enabled: true
          reload_interval: 60
          macos:
            - query: process == "sshd" OR message CONTAINS "invalid"
              level: info
            - query: process == "wazuh-agent"
              type: log
        )"
    )
);
