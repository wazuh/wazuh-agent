#include <gtest/gtest.h>
#include "logcollector_mock.hpp"
#include <configuration_parser.hpp>
#include <file_reader.hpp>
#include "tempfile.hpp"

using namespace configuration;
using namespace logcollector;

TEST(Logcollector, AddReader) {
    auto logcollector = LogcollectorMock();
    auto a = TempFile("/tmp/A.log");
    auto fileReader = std::make_shared<FileReader>(logcollector, "/tmp/*.log");

    EXPECT_CALL(logcollector, EnqueueTask(::testing::_)).Times(1);
    EXPECT_CALL(logcollector, AddReader(::testing::_));

    logcollector.AddReader(fileReader);
}

TEST(Logcollector, SetupFileReader) {
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
    auto config = ConfigurationParser(std::string(CONFIG_RAW));

    EXPECT_CALL(logcollector, AddReader(::testing::_)).Times(2)
        .WillOnce(::testing::SaveArg<0>(&capturedReader1))
        .WillOnce(::testing::SaveArg<0>(&capturedReader2));

    logcollector.SetupFileReader(config);

    ASSERT_NE(capturedReader1, nullptr);
    ASSERT_NE(capturedReader2, nullptr);
}
