#include <gtest/gtest.h>
#include <list>
#include <spdlog/spdlog.h>
#include <sstream>

#include <file_reader.hpp>
#include <logcollector.hpp>
#include <logcollector_mock.hpp>
#include <tempfile.hpp>

using namespace logcollector;

class MockCallback
{
public:
    MOCK_METHOD(void, Call, (const std::string&), ());
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
    try
    {
        auto lf = Localfile("unexisting.file");
        FAIL() << "Expected OpenError";
    }
    catch (OpenError& err)
    {
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

    try
    {
        lf.Rotated();
        FAIL() << "Expected OpenError";
    }
    catch (OpenError& err)
    {
        ASSERT_STREQ(err.what(), "Cannot open file: /tmp/A.log");
    }
}

TEST(FileReader, Reload)
{
    spdlog::default_logger()->sinks().clear();
    MockCallback mockCallback;

    EXPECT_CALL(mockCallback, Call("/tmp/fileA.log")).Times(1);
    EXPECT_CALL(mockCallback, Call("/tmp/fileB.log")).Times(1);
    EXPECT_CALL(mockCallback, Call("/tmp/fileC.log")).Times(1);
    EXPECT_CALL(mockCallback, Call("/tmp/fileD.log")).Times(1);

    auto a = TempFile("/tmp/fileA.log");
    auto b = TempFile("/tmp/fileB.log");
    auto c = TempFile("/tmp/fileC.log");

    auto regex = "/tmp/file*.log";
    Logcollector logcollector;
    FileReader reader(logcollector, regex, 500, 60000); // NOLINT
    reader.Reload([&](Localfile& lf) { mockCallback.Call(lf.Filename()); });

    auto d = TempFile("/tmp/fileD.log");
    reader.Reload([&](Localfile& lf) { mockCallback.Call(lf.Filename()); });
}
