#include <gtest/gtest.h>
#include <sstream>
#include <list>
#include <spdlog/spdlog.h>

#include <logcollector.hpp>
#include <file_reader.hpp>
#include "queue_mock.hpp"
#include "tempfile.hpp"
#include "logcollector_mock.hpp"

using namespace logcollector;

class MockCallback {
public:
    MOCK_METHOD(void, Call, (const std::string &), ());
};

TEST(Localfile, FullLine)
{
    auto stream = std::make_shared<std::stringstream>();
    auto lf = Localfile(stream);

    *stream << "Hello World\n";
    auto answer = lf.NextLog();
    ASSERT_EQ(answer, "Hello World");
}

TEST(Localfile, PartialLine)
{
    auto stream = std::make_shared<std::stringstream>();
    auto lf = Localfile(stream);

    *stream << "Hello World";
    auto answer = lf.NextLog();
    ASSERT_EQ(answer, "");

    *stream << "\n";
    answer = lf.NextLog();
    ASSERT_EQ(answer, "Hello World");
}

TEST(Localfile, OpenError)
{
    try {
        auto lf = Localfile("unexisting.file");
        FAIL() << "Expected OpenError";
    } catch (OpenError & err) {
        ASSERT_STREQ(err.what(), "Cannot open file: unexisting.file");
    }
}

TEST(Localfile, Rotated)
{
    auto fileA = TempFile("/tmp/A.log", "Hello World");
    auto lf = Localfile("/tmp/A.log");

    lf.SeekEnd();
    ASSERT_FALSE(lf.Rotated());

    fileA.Truncate();
    ASSERT_TRUE(lf.Rotated());
}

TEST(Localfile, Deleted)
{
    auto fileA = std::make_unique<TempFile>("/tmp/A.log", "Hello World");
    auto lf = Localfile("/tmp/A.log");

    fileA.reset();

    try {
        lf.Rotated();
        FAIL() << "Expected OpenError";
    } catch (OpenError & err) {
        ASSERT_STREQ(err.what(), "Cannot open file: /tmp/A.log");
    }
}

TEST(FileReader, Reload) {
    spdlog::default_logger()->sinks().clear();
    MockCallback mockCallback;

    EXPECT_CALL(mockCallback, Call("/tmp/A.log")).Times(1);
    EXPECT_CALL(mockCallback, Call("/tmp/B.log")).Times(1);
    EXPECT_CALL(mockCallback, Call("/tmp/C.log")).Times(1);
    EXPECT_CALL(mockCallback, Call("/tmp/D.log")).Times(1);

    auto a = TempFile("/tmp/A.log");
    auto b = TempFile("/tmp/B.log");
    auto c = TempFile("/tmp/C.log");

    FileReader reader(Logcollector::Instance(), "/tmp/*.log");
    reader.Reload([&](Localfile& lf) { mockCallback.Call(lf.Filename()); });

    auto d = TempFile("/tmp/D.log");
    reader.Reload([&](Localfile& lf) { mockCallback.Call(lf.Filename()); });
}
