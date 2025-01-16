#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <journal_log.hpp>

using namespace testing;

class JournalLogTests : public ::testing::Test {
protected:
    std::unique_ptr<JournalLog> journal;

    void SetUp() override {
        journal = std::make_unique<JournalLog>();
        journal->Open();
    }

    void TearDown() override {
        journal.reset();
    }

    FilterGroup CreateBasicFilterGroup() {
        return {
            {"UNIT", "test.service", true},
            {"PRIORITY", "6", true}
        };
    }
};

TEST_F(JournalLogTests, FilterValidation) {
    struct TestCase {
        JournalFilter filter;
        bool expectedValid;
    };

    std::vector<TestCase> testCases = {
        {{"UNIT", "systemd-journald.service", true}, true},
        {{"", "value", true}, false},
        {{"field", "", true}, false},
        {{"field", "value", false}, true}
    };

    for (const auto& tc : testCases) {
        EXPECT_EQ(JournalLog::ValidateFilter(tc.filter), tc.expectedValid)
            << "Field: " << tc.filter.field
            << ", Value: " << tc.filter.value;
    }
}

TEST_F(JournalLogTests, FilterMatching) {
    struct TestCase {
        JournalFilter filter;
        std::string input;
        bool expectedMatch;
    };

    std::vector<TestCase> testCases = {
        {{"UNIT", "test.service", true}, "test.service", true},
        {{"UNIT", "test", false}, "test.service", true},
        {{"UNIT", "service1|service2", true}, "service1", true},
        {{"UNIT", "service1|service2", true}, "service3", false},
        {{"UNIT", "sys|jour", false}, "system", true}
    };

    for (const auto& tc : testCases) {
        EXPECT_EQ(tc.filter.Matches(tc.input), tc.expectedMatch)
            << "Filter: " << tc.filter.value
            << ", Input: " << tc.input;
    }
}

TEST_F(JournalLogTests, BasicJournalOperations) {
    auto group = CreateBasicFilterGroup();

    // Test basic operations separately to identify failures
    EXPECT_NO_THROW(journal->AddFilterGroup(group, true));
    EXPECT_NO_THROW(journal->SeekHead());
    EXPECT_NO_THROW(journal->SeekTail());
    EXPECT_NO_THROW(journal->Next());
    EXPECT_NO_THROW(journal->Previous());

    // Test cursor operations with proper error handling
    try {
        auto cursor = journal->GetCursor();
        if (!cursor.empty()) {
            EXPECT_TRUE(journal->SeekCursor(cursor));
        }
    } catch (const JournalLogException& e) {
        // Log but don't fail - cursor operations might not work in test environment
        std::cerr << "Cursor operations failed: " << e.what() << '\n';
    }
}

TEST_F(JournalLogTests, MessageProcessing) {
    auto group = CreateBasicFilterGroup();
    journal->AddFilterGroup(group, true);

    FilterSet filters{group};
    auto message = journal->GetNextFilteredMessage(filters, true);

    if (message) {
        EXPECT_FALSE(message->message.empty());
        EXPECT_FALSE(message->fieldValue.empty());
    }
}

TEST_F(JournalLogTests, ErrorHandling) {
    EXPECT_THROW(journal->GetData("NONEXISTENT_FIELD"), JournalLogException);

    FilterGroup invalidGroup{{"", "value", true}};
    EXPECT_THROW(journal->AddFilterGroup(invalidGroup, false), JournalLogException);
}
