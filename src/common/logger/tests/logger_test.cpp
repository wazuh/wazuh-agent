#include <logger.hpp>

#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>

#ifdef _WIN32
#include <spdlog/sinks/win_eventlog_sink.h>
#else // __linux__ || __APPLE__
#include <spdlog/sinks/stdout_color_sinks.h>
#endif

#include <memory>
#include <sstream>

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

#ifdef _WIN32
TEST_F(LoggerConstructorTest, WindowsLoggerConstructor)
{
    auto current_logger = spdlog::default_logger();
    EXPECT_EQ(current_logger->name(), "wazuh-agent");

    EXPECT_EQ(spdlog::get_level(), spdlog::level::info);

    auto sinks = current_logger->sinks();
    ASSERT_FALSE(sinks.empty());
    auto win_sink = std::dynamic_pointer_cast<spdlog::sinks::win_eventlog_sink_mt>(sinks[0]);
    EXPECT_NE(win_sink, nullptr);
}
#else  // __linux__ || __APPLE__
TEST_F(LoggerConstructorTest, UnixLoggerConstructor)
{
    auto current_logger = spdlog::default_logger();
    EXPECT_EQ(current_logger->name(), "wazuh-agent");

    EXPECT_EQ(spdlog::get_level(), spdlog::level::info);

    auto sinks = current_logger->sinks();
    ASSERT_FALSE(sinks.empty());
    auto sink = std::dynamic_pointer_cast<spdlog::sinks::stderr_color_sink_mt>(sinks[0]);
    EXPECT_NE(sink, nullptr);
}

TEST_F(LoggerConstructorTest, UnixLoggerConstructorFail)
{
    auto current_logger = spdlog::default_logger();
    EXPECT_EQ(current_logger->name(), "wazuh-agent");

    EXPECT_EQ(spdlog::get_level(), spdlog::level::info);

    auto sinks = current_logger->sinks();
    ASSERT_FALSE(sinks.empty());
    auto sink = std::dynamic_pointer_cast<spdlog::sinks::stdout_color_sink_mt>(sinks[0]);
    EXPECT_EQ(sink, nullptr);
}
#endif // __linux__ || __APPLE__

TEST_F(LoggerMessageTest, LogsTraceMessage)
{
    LogTrace("This is a trace message");
    const std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[trace]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is a trace message") != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsDebugMessage)
{
    LogDebug("This is a debug message");
    const std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[debug]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is a debug message") != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsInfoMessage)
{
    LogInfo("This is an info message");
    const std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[info]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is an info message") != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsWarnMessage)
{
    LogWarn("This is a warning message");
    const std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[warning]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is a warning message") != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsErrorMessage)
{
    std::string error_message = "Can't open database";
    const std::exception e;
    LogError("{}: {}", error_message, e.what());
    const std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[error]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("Can't open database") != std::string::npos);
    EXPECT_TRUE(logged_message.find(e.what()) != std::string::npos);
}

TEST_F(LoggerMessageTest, LogsCriticalMessage)
{
    LogCritical("This is a critical message");
    const std::string logged_message = oss.str();
    EXPECT_TRUE(logged_message.find("[critical]") != std::string::npos);
    EXPECT_TRUE(logged_message.find("This is a critical message") != std::string::npos);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
