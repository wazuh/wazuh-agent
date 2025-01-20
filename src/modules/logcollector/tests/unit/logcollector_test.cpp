#include "logcollector_mock.hpp"
#include "tempfile.hpp"
#include <configuration_parser.hpp>
#include <file_reader.hpp>
#include <gtest/gtest.h>
#include <regex>

using namespace configuration;
using namespace logcollector;

static bool IsISO8601(const std::string& datetime)
{
    const std::regex iso8601Regex(R"(^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:Z|[+-]\d{2}:\d{2})$)");
    return std::regex_match(datetime, iso8601Regex);
}

TEST(Logcollector, AddReader)
{
    auto logcollector = LogcollectorMock();
    auto a = TempFile("/tmp/A.log");
    auto fileReader = std::make_shared<FileReader>(logcollector, "/tmp/*.log", 500, 60000); // NOLINT

    EXPECT_CALL(logcollector, EnqueueTask(::testing::_)).Times(1);
    EXPECT_CALL(logcollector, AddReader(::testing::_));

    logcollector.AddReader(fileReader);
}

TEST(Logcollector, SetupFileReader)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      localfiles:
        - /var/log/auth.log
        - /var/log/syslog
      reload_interval: 60
      file_wait: 500
    )";

    std::shared_ptr<IReader> capturedReader1;
    std::shared_ptr<IReader> capturedReader2;
    auto logcollector = LogcollectorMock();
    auto config = std::make_shared<configuration::ConfigurationParser>(std::string(CONFIG_RAW));

    EXPECT_CALL(logcollector, AddReader(::testing::_))
        .Times(2)
        .WillOnce(::testing::SaveArg<0>(&capturedReader1))
        .WillOnce(::testing::SaveArg<0>(&capturedReader2));

    logcollector.SetupFileReader(config);

    ASSERT_NE(capturedReader1, nullptr);
    ASSERT_NE(capturedReader2, nullptr);
}

TEST(Logcollector, SendMessage)
{
    PushMessageMock mock;
    LogcollectorMock logcollector;

    logcollector.SetPushMessageFunction([&mock](Message message) { return mock.Call(std::move(message)); });

    Message capturedMessage(MessageType::STATELESS, nlohmann::json::object(), "", "", "");

    EXPECT_CALL(mock, Call(::testing::_))
        .WillOnce(::testing::DoAll(::testing::SaveArg<0>(&capturedMessage), ::testing::Return(0)));

    const auto MODULE = "logcollector";
    const auto LOCATION = "/test/location";
    const auto LOG = "test log";
    const auto PROVIDER = "syslog";
    const auto METADATA = R"({"module":"logcollector","type":"reader"})";

    logcollector.SendMessage(LOCATION, LOG, "reader");

    ASSERT_EQ(capturedMessage.type, MessageType::STATELESS);
    ASSERT_EQ(capturedMessage.data["log"]["file"]["path"], LOCATION);
    ASSERT_EQ(capturedMessage.data["event"]["original"], LOG);
    ASSERT_TRUE(IsISO8601(capturedMessage.data["event"]["created"]));
    ASSERT_EQ(capturedMessage.data["event"]["module"], MODULE);
    ASSERT_EQ(capturedMessage.data["event"]["provider"], PROVIDER);
    ASSERT_EQ(capturedMessage.metaData, METADATA);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
