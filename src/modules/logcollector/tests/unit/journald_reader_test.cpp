#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "journald_reader.hpp"
#include "logcollector_mock.hpp"

using namespace logcollector;
using namespace testing;

class JournaldReaderTests : public ::testing::Test {
protected:
    LogcollectorMock logcollector;
    FilterGroup testFilters;
    bool ignoreIfMissing{true};
    std::time_t fileWait{1000};
    static constexpr size_t m_extraLength = 100;

    void SetUp() override {
        testFilters = {
            {"UNIT", "test.service", true},
            {"PRIORITY", "6", true}
        };
    }

    JournaldReader CreateReader() {
        return {logcollector, testFilters, ignoreIfMissing, fileWait};
    }
};

TEST_F(JournaldReaderTests, BasicOperations) {
    auto reader = CreateReader();

    EXPECT_THAT(reader.GetFilterDescription(),
        ::testing::AllOf(
            ::testing::HasSubstr("2 conditions"),
            ::testing::HasSubstr("UNIT"),
            ::testing::HasSubstr("PRIORITY")
        ));

    EXPECT_NO_THROW(reader.Stop());
}

TEST_F(JournaldReaderTests, MessageProcessing) {
    auto reader = CreateReader();

    ON_CALL(logcollector, MockWait).WillByDefault(Return());

    auto runTask = reader.Run();
    reader.Stop();
}

TEST_F(JournaldReaderTests, FilterHandling) {
    struct TestCase {
        FilterGroup filters;
        std::string expectedDesc;
    };

    std::vector<TestCase> testCases = {
        {{}, "0 conditions"},
        {{{"UNIT", "service1|service2", true}}, "1 conditions"},
        {{{"UNIT", "service1", true}, {"PRIORITY", "3|4|5", true}}, "2 conditions"}
    };

    for (const auto& tc : testCases) {
        JournaldReader reader(logcollector, tc.filters, ignoreIfMissing, fileWait);
        EXPECT_THAT(reader.GetFilterDescription(), ::testing::HasSubstr(tc.expectedDesc));
    }
}

TEST_F(JournaldReaderTests, MessageTruncation) {
    auto reader = CreateReader();

    auto runTask = reader.Run();
    reader.Stop();
}
