#include <gtest/gtest.h>
#include <sstream>
#include <logcollector.hpp>
#include <file_reader.hpp>
#include "queue_mock.hpp"

using namespace std;
using namespace logcollector;

TEST(Localfile, FullLine)
{
    auto stream = make_shared<stringstream>();
    auto lf = Localfile(stream);

    *stream << "Hello World\n";
    auto answer = lf.NextLog();
    ASSERT_EQ(answer, "Hello World");
}

TEST(Localfile, PartialLine)
{
    auto stream = make_shared<stringstream>();
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
