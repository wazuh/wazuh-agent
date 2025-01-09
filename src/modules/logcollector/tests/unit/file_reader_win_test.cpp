#include <gtest/gtest.h>
#include <sstream>
#include <list>
#include <spdlog/spdlog.h>

#include <logcollector.hpp>
#include <file_reader.hpp>
#include "tempfile.hpp"
#include "logcollector_mock.hpp"

using namespace logcollector;

static constexpr auto TMP_FILE_DIR = "C:\\Temp\\";

inline std::string GetFullFileName(const std::string& filename) {
    //TODO: move to setup stage of test only for windows
    std::filesystem::create_directories(TMP_FILE_DIR);
    return TMP_FILE_DIR + filename;
}

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
    auto fileA = TempFile(GetFullFileName("A.log"), "Hello World");
    auto lf = Localfile(GetFullFileName("A.log"));

    lf.SeekEnd();
    ASSERT_FALSE(lf.Rotated());

    fileA.Truncate();
    ASSERT_TRUE(lf.Rotated());
}

TEST(Localfile, Deleted)
{
    //FIXME: The process cannot access the file because it is being used by another process.
    GTEST_SKIP();
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

    EXPECT_CALL(mockCallback, Call(GetFullFileName("A.log"))).Times(1);
    EXPECT_CALL(mockCallback, Call(GetFullFileName("B.log"))).Times(1);
    EXPECT_CALL(mockCallback, Call(GetFullFileName("C.log"))).Times(1);
    EXPECT_CALL(mockCallback, Call(GetFullFileName("D.log"))).Times(1);

    auto a = TempFile(GetFullFileName("A.log"));
    auto b = TempFile(GetFullFileName("B.log"));
    auto c = TempFile(GetFullFileName("C.log"));

    auto regex = TMP_FILE_DIR + std::string("*.log");
    FileReader reader(Logcollector::Instance(), regex, 500, 60000); //NOLINT
    reader.Reload([&](Localfile& lf) { mockCallback.Call(lf.Filename()); });

    auto d = TempFile(GetFullFileName("D.log"));
    reader.Reload([&](Localfile& lf) { mockCallback.Call(lf.Filename()); });
}
