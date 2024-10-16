#include <gtest/gtest.h>
#include "logcollector_mock.hpp"
#include <file_reader.hpp>
#include "tempfile.hpp"

using namespace logcollector;
using namespace std;

TEST(Logcollector, AddReader) {
    auto logcollector = LogcollectorMock();
    auto a = TempFile("/tmp/A.log");
    auto fileReader = make_shared<FileReader>(logcollector, "/tmp/*.log");

    EXPECT_CALL(logcollector, EnqueueTask(::testing::_)).Times(1);

    logcollector.AddReader(fileReader);
}
