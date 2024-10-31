#include <logger.hpp>

#include <gtest/gtest.h>

#include <spdlog/sinks/ostream_sink.h>

#ifdef __APPLE__
#include <spdlog/sinks/syslog_sink.h>
#endif

#ifdef __linux__
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#endif

#ifdef _WIN32
#include <spdlog/sinks/win_eventlog_sink.h>
#endif

#include <sstream>
#include <memory>
#include <string>

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

class LoggerMessageTest : public ::testing::Test
{
protected:
    std::ostringstream oss;
    std::shared_ptr<spdlog::sinks::ostream_sink_st> ostream_sink;

    void SetUp() override
    {
        oss.str("");
        ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_st>(oss);
        auto logger = std::make_shared<spdlog::logger>("test_logger", ostream_sink);
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::trace);
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
    auto sink = std::dynamic_pointer_cast<spdlog::sinks::stderr_color_sink_mt>(sinks[0]);
    EXPECT_NE(sink, nullptr);
}

TEST_F(LoggerConstructorTest, LinuxLoggerConstructorFail)
{
    auto current_logger = spdlog::default_logger();
    EXPECT_EQ(current_logger->name(), "wazuh-agent");

    EXPECT_EQ(spdlog::get_level(), spdlog::level::trace);

    auto sinks = current_logger->sinks();
    ASSERT_FALSE(sinks.empty());
    auto sink = std::dynamic_pointer_cast<spdlog::sinks::stdout_color_sink_mt>(sinks[0]);
    EXPECT_EQ(sink, nullptr);
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

TEST_F(LoggerMessageTest, LogsTraceMessage)
{
    LogTrace("This is a trace message");
    std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[TRACE]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is a trace message") != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsDebugMessage)
{
    LogDebug("This is a debug message");
    std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[DEBUG]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is a debug message") != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsInfoMessage)
{
    LogInfo("This is an info message");
    std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[INFO]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is an info message") != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsWarnMessage)
{
    LogWarn("This is a warning message");
    std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[WARN]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is a warning message") != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsErrorMessage)
{
    std::string error_message = "Can't open database";
    std::exception e;
    LogError("{}: {}", error_message, e.what());
    std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[ERROR]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("Can't open database") != std::string::npos);
    EXPECT_TRUE(logged_message.find(e.what()) != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsCriticalMessage)
{
    LogCritical("This is a critical message");
    std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[CRITICAL]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is a critical message") != std::string::npos);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
