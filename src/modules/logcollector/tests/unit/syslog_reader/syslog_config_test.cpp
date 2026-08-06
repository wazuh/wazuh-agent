#include "logcollector_mock.hpp"

#include <configuration_parser.hpp>
#include <gtest/gtest.h>

#include <memory>
#include <string>

using namespace configuration;
using namespace logcollector;

namespace
{
    /// @brief Runs SetupSyslogReaders against the given YAML and returns how many readers were created
    int CountSyslogReaders(const std::string& yaml)
    {
        auto logcollector = LogcollectorMock();
        auto config = std::make_shared<ConfigurationParser>(yaml);

        int readerCount = 0;
        EXPECT_CALL(logcollector, AddReader(::testing::_))
            .WillRepeatedly(::testing::Invoke([&readerCount](std::shared_ptr<IReader>) { ++readerCount; }));

        logcollector.SetupSyslogReaders(config);
        return readerCount;
    }
} // namespace

TEST(SyslogConfig, NoSyslogSectionCreatesNoListeners)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      localfiles:
        - /var/log/auth.log
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 0);
}

TEST(SyslogConfig, UdpListenerParses)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          bind_address: 127.0.0.1
          port: 5514
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 1);
}

TEST(SyslogConfig, TcpListenerParses)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: tcp
          bind_address: 127.0.0.1
          port: 1514
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 1);
}

TEST(SyslogConfig, BindAddressIsOptional)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          port: 5514
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 1);
}

TEST(SyslogConfig, MultipleListenersParse)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          bind_address: 127.0.0.1
          port: 5514
        - protocol: tcp
          bind_address: 127.0.0.1
          port: 1514
        - protocol: udp
          bind_address: 127.0.0.1
          port: 5515
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 3);
}

TEST(SyslogConfig, UnsupportedProtocolIsRejected)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: http
          bind_address: 127.0.0.1
          port: 5514
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 0);
}

TEST(SyslogConfig, MissingProtocolIsRejected)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - bind_address: 127.0.0.1
          port: 5514
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 0);
}

TEST(SyslogConfig, OutOfRangePortIsRejected)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          bind_address: 127.0.0.1
          port: 99999
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 0);
}

TEST(SyslogConfig, ZeroPortIsRejected)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          bind_address: 127.0.0.1
          port: 0
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 0);
}

TEST(SyslogConfig, NonNumericPortIsRejected)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: tcp
          bind_address: 127.0.0.1
          port: abc
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 0);
}

TEST(SyslogConfig, MissingPortIsRejected)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          bind_address: 127.0.0.1
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 0);
}

TEST(SyslogConfig, InvalidBindAddressIsRejected)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          bind_address: 999.999.999.999
          port: 5514
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 0);
}

TEST(SyslogConfig, DuplicateListenerIsRejected)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          bind_address: 127.0.0.1
          port: 5514
        - protocol: udp
          bind_address: 127.0.0.1
          port: 5514
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 1);
}

TEST(SyslogConfig, SamePortDifferentProtocolIsAllowed)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          bind_address: 127.0.0.1
          port: 1514
        - protocol: tcp
          bind_address: 127.0.0.1
          port: 1514
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 2);
}

TEST(SyslogConfig, ValidAndInvalidListenersAreHandledIndependently)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      syslog:
        - protocol: udp
          bind_address: 127.0.0.1
          port: 5514
        - protocol: sctp
          bind_address: 127.0.0.1
          port: 5515
        - protocol: tcp
          bind_address: 127.0.0.1
          port: 1514
    )";

    EXPECT_EQ(CountSyslogReaders(CONFIG_RAW), 2);
}
