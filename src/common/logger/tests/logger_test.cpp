#include <logger.hpp>

#include <gtest/gtest.h>
#ifdef __APPLE__
#include <spdlog/sinks/syslog_sink.h>
#endif
#ifdef __linux__
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/systemd_sink.h>
#endif
#ifdef _WIN32
#include <spdlog/sinks/win_eventlog_sink.h>
#endif

class LoggerConstructorTest : public ::testing::Test
{
protected:
    std::unique_ptr<Logger> logger;

    void SetUp() override
    {
        logger = std::make_unique<Logger>();
    }

    void TearDown() override
    {
        spdlog::drop_all();
    }
};

#ifdef __linux__
TEST_F(LoggerConstructorTest, LinuxLoggerConstructor)
{
    auto current_logger = spdlog::default_logger();
    EXPECT_EQ(current_logger->name(), "wazuh-agent");

    EXPECT_EQ(spdlog::get_level(), spdlog::level::trace);

    auto sinks = current_logger->sinks();
    ASSERT_FALSE(sinks.empty());
    auto systemd_sink = std::dynamic_pointer_cast<spdlog::sinks::systemd_sink_st>(sinks[0]);
    EXPECT_NE(systemd_sink, nullptr);
}

TEST_F(LoggerConstructorTest, LinuxLoggerConstructorFail)
{
    auto current_logger = spdlog::default_logger();
    EXPECT_EQ(current_logger->name(), "wazuh-agent");

    EXPECT_EQ(spdlog::get_level(), spdlog::level::trace);

    auto sinks = current_logger->sinks();
    ASSERT_FALSE(sinks.empty());
    auto systemd_sink = std::dynamic_pointer_cast<spdlog::sinks::syslog_sink_mt>(sinks[0]);
    EXPECT_EQ(systemd_sink, nullptr);
}
#endif // __linux__

#ifdef _WIN32
TEST_F(LoggerConstructorTest, WindowsLoggerConstructor)
{
    auto current_logger = spdlog::default_logger();
    EXPECT_EQ(current_logger->name(), "wazuh-agent");

    EXPECT_EQ(spdlog::get_level(), spdlog::level::trace);

    auto sinks = current_logger->sinks();
    ASSERT_FALSE(sinks.empty());
    auto win_sink = std::dynamic_pointer_cast<spdlog::sinks::win_eventlog_sink_st>(sinks[0]);
    EXPECT_NE(win_sink, nullptr);
}
#endif // _WIN32

#ifdef __APPLE__
TEST_F(LoggerConstructorTest, MacOSLoggerConstructor)
{
    auto current_logger = spdlog::default_logger();
    EXPECT_EQ(current_logger->name(), "wazuh-agent");

    EXPECT_EQ(spdlog::get_level(), spdlog::level::trace);

    auto sinks = current_logger->sinks();
    ASSERT_FALSE(sinks.empty());
    auto syslog_sink = std::dynamic_pointer_cast<spdlog::sinks::syslog_sink_mt>(sinks[0]);
    EXPECT_NE(syslog_sink, nullptr);
}
#endif // __APPLE__

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
