#include "../src/oslogstore_dependencies.hpp"
#include "../src/oslogstore_impl.hpp"

#include <oslogstore.hpp>

#import <Foundation/Foundation.h>
#import <OSLog/OSLog.h>

#include <exception>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

/// MockOSLogEntry is a mock class that inherits from OSLogEntry and is used to create mock log entries.
@interface MockOSLogEntry : OSLogEntry
@property(nonatomic, assign) NSInteger level;
@end

@interface MockOSLogEntry ()
@property(nonatomic, strong, readwrite) NSDate* date;
@property(nonatomic, copy, readwrite) NSString* composedMessage;
@end

@implementation MockOSLogEntry

@synthesize date = _date;
@synthesize composedMessage = _composedMessage;

@end

class MockIOSLogEntries : public IOSLogEntries
{
public:
    MockIOSLogEntries(NSArray<OSLogEntry*>* entries)
        : m_entries(entries)
    {
        [m_entries retain];
    }

    ~MockIOSLogEntries() override
    {
        [m_entries release];
    }

    NSArray<OSLogEntry*>* AllEntries() override
    {
        return m_entries;
    }

private:
    NSArray<OSLogEntry*>* m_entries;
};

class MockIOSLogStore : public IOSLogStore
{
public:
    MockIOSLogStore(NSArray<OSLogEntry*>* entries = nil, bool shouldFail = false, std::string error = "")
        : m_entries(entries)
        , m_shouldFail(shouldFail)
        , m_error(error)
    {
    }

    std::unique_ptr<IOSLogEntries> GetOSLogEntries([[maybe_unused]] NSDate* startDate,
                                                   [[maybe_unused]] NSPredicate* predicate,
                                                   std::string& error) override
    {
        if (m_shouldFail)
        {
            error = m_error;
            return nullptr;
        }
        return std::make_unique<MockIOSLogEntries>(m_entries);
    }

private:
    NSArray<OSLogEntry*>* m_entries;
    bool m_shouldFail;
    std::string m_error;
};

class MockILogProcessor : public ILogProcessor
{
public:
    MockILogProcessor(std::vector<IOSLogStoreWrapper::LogEntry> processedEntries = {})
        : m_processedEntries(processedEntries)
    {
    }

    ~MockILogProcessor() override = default;

    std::vector<IOSLogStoreWrapper::LogEntry>
    ProcessEntries([[maybe_unused]] NSArray<OSLogEntry*>* entries,
                   [[maybe_unused]] IOSLogStoreWrapper::LogLevel logLevel) override
    {
        return m_processedEntries;
    }

private:
    std::vector<IOSLogStoreWrapper::LogEntry> m_processedEntries;
};

class OSLogStoreWrapperImplTest : public ::testing::Test
{
public:
    OSLogStoreWrapperImplTest()
    {
        log1 = [[MockOSLogEntry alloc] init];
        log1.date = [NSDate dateWithTimeIntervalSince1970:aPointInTime];
        log1.composedMessage = [NSString stringWithUTF8String:anErrorMsg.c_str()];
        log1.level = OSLogStoreWrapper::LogLevel::Error;
        [log1 retain];

        log2 = [[MockOSLogEntry alloc] init];
        log2.date = [NSDate dateWithTimeIntervalSince1970:1672538400.0];
        log2.composedMessage = [NSString stringWithUTF8String:anInfoMsg.c_str()];
        log2.level = OSLogStoreWrapper::LogLevel::Info;
        [log2 retain];

        std::vector<MockOSLogEntry*> oneEntryVector = {log1};
        oneEntryNSArray = CreateNSArray(oneEntryVector);
        [oneEntryNSArray retain];

        std::vector<MockOSLogEntry*> entryVector = {log1, log2};
        twoEntriesNSArray = CreateNSArray(entryVector);
        [twoEntriesNSArray retain];

        IOSLogStoreWrapper::LogEntry processedEntry = {aPointInTime, aDate, anErrorMsg};
        vectorWithOneEntryProcessedAsExpected = {processedEntry};

        IOSLogStoreWrapper::LogEntry processedEntry2 = {1672538400.0, anotherDate, anInfoMsg};
        vectorWithTwoEntriesProcessedAsExpected = {processedEntry, processedEntry2};
    }

    ~OSLogStoreWrapperImplTest()
    {
        [oneEntryNSArray release];
        [log2 release];
        [log1 release];
    }

protected:
    NSArray<MockOSLogEntry*>* CreateNSArray(const std::vector<MockOSLogEntry*>& entries)
    {
        return [NSArray arrayWithObjects:entries.data() count:static_cast<NSUInteger>(entries.size())];
    }

    const double aPointInTime = 1672534800.0;
    const double anotherPointInTime = 1672538400.0;
    const double anEarlierPointInTime = 1672530000.0;

    const std::string aDate = "2023-01-01 00:00:00";
    const std::string anotherDate = "2023-01-01 01:00:00";
    const std::string aQuery = "processImagePath == '/Applications/MockApp.app'";
    const std::string anInfoMsg = "Info log entry";
    const std::string anErrorMsg = "Error log entry";

    MockOSLogEntry* log1;
    MockOSLogEntry* log2;
    NSArray<MockOSLogEntry*>* oneEntryNSArray;
    NSArray<MockOSLogEntry*>* twoEntriesNSArray;

    std::vector<IOSLogStoreWrapper::LogEntry> vectorWithOneEntryProcessedAsExpected;
    std::vector<IOSLogStoreWrapper::LogEntry> vectorWithTwoEntriesProcessedAsExpected;
};

TEST_F(OSLogStoreWrapperImplTest, AllEntries_Success)
{
    auto mockStore = std::make_unique<MockIOSLogStore>(oneEntryNSArray);
    auto mockProcessor = std::make_unique<MockILogProcessor>(vectorWithOneEntryProcessedAsExpected);

    // Initialize OSLogStoreWrapperImpl and inject mocks
    OSLogStoreWrapperImpl impl;
    impl.SetLogStore(std::move(mockStore));
    impl.SetLogProcessor(std::move(mockProcessor));

    std::vector<IOSLogStoreWrapper::LogEntry> result;
    EXPECT_NO_THROW({ result = impl.AllEntries(anEarlierPointInTime, aQuery, IOSLogStoreWrapper::LogLevel::Error); });
    EXPECT_EQ(result.size(), 1);
    EXPECT_DOUBLE_EQ(result[0].dateInSeconds, aPointInTime);
    EXPECT_EQ(result[0].date, aDate);
    EXPECT_EQ(result[0].log, anErrorMsg);
}

// Test Case 2: GetOSLogEntries fails and throws runtime_error
TEST_F(OSLogStoreWrapperImplTest, AllEntries_GetOSLogEntriesFailure)
{
    auto failingMockStore = std::make_unique<MockIOSLogStore>(nullptr, true, "Mock failure to get OSLogEntries");
    auto mockProcessor = std::make_unique<MockILogProcessor>();

    // Initialize OSLogStoreWrapperImpl and inject mocks
    OSLogStoreWrapperImpl impl;
    impl.SetLogStore(std::move(failingMockStore));
    impl.SetLogProcessor(std::move(mockProcessor));

    EXPECT_THROW(
        {
            std::vector<IOSLogStoreWrapper::LogEntry> result =
                impl.AllEntries(anEarlierPointInTime, aQuery, IOSLogStoreWrapper::LogLevel::Info);
        },
        std::runtime_error);
}

TEST_F(OSLogStoreWrapperImplTest, AllEntries_NoLogEntries)
{
    NSArray<MockOSLogEntry*>* emptyEntries = [NSArray array];
    auto emptyMockStore = std::make_unique<MockIOSLogStore>(emptyEntries);
    auto emptyMockProcessor = std::make_unique<MockILogProcessor>(std::vector<IOSLogStoreWrapper::LogEntry> {});

    OSLogStoreWrapperImpl impl;
    impl.SetLogStore(std::move(emptyMockStore));
    impl.SetLogProcessor(std::move(emptyMockProcessor));

    std::vector<IOSLogStoreWrapper::LogEntry> result;
    EXPECT_NO_THROW({ result = impl.AllEntries(anEarlierPointInTime, aQuery, IOSLogStoreWrapper::LogLevel::Info); });
    EXPECT_TRUE(result.empty());
}

// Test Case 4: ProcessEntries returns multiple entries
TEST_F(OSLogStoreWrapperImplTest, AllEntries_MultipleLogEntries)
{
    auto mockStore = std::make_unique<MockIOSLogStore>(twoEntriesNSArray);
    auto mockProcessor = std::make_unique<MockILogProcessor>(vectorWithTwoEntriesProcessedAsExpected);

    // Initialize OSLogStoreWrapperImpl and inject mocks
    OSLogStoreWrapperImpl impl;
    impl.SetLogStore(std::move(mockStore));
    impl.SetLogProcessor(std::move(mockProcessor));

    std::vector<IOSLogStoreWrapper::LogEntry> result;
    EXPECT_NO_THROW({ result = impl.AllEntries(anEarlierPointInTime, aQuery, IOSLogStoreWrapper::LogLevel::Info); });

    EXPECT_EQ(result.size(), 2);
    EXPECT_DOUBLE_EQ(result[0].dateInSeconds, aPointInTime);
    EXPECT_EQ(result[0].date, aDate);
    EXPECT_EQ(result[0].log, anErrorMsg);
    EXPECT_DOUBLE_EQ(result[1].dateInSeconds, anotherPointInTime);
    EXPECT_EQ(result[1].date, anotherDate);
    EXPECT_EQ(result[1].log, anInfoMsg);
}

// Test Case 5: ProcessEntries with logLevel higher than any entry
TEST_F(OSLogStoreWrapperImplTest, AllEntries_LogLevelHigher)
{
    auto mockStore = std::make_unique<MockIOSLogStore>(oneEntryNSArray);

    // Empty MockILogProcessor with no processed entries due to higher logLevel
    auto mockProcessor = std::make_unique<MockILogProcessor>(std::vector<IOSLogStoreWrapper::LogEntry> {});

    // Initialize OSLogStoreWrapperImpl and inject mocks
    OSLogStoreWrapperImpl impl;
    impl.SetLogStore(std::move(mockStore));
    impl.SetLogProcessor(std::move(mockProcessor));

    std::vector<IOSLogStoreWrapper::LogEntry> result;
    EXPECT_NO_THROW({ result = impl.AllEntries(anEarlierPointInTime, aQuery, IOSLogStoreWrapper::LogLevel::Fault); });
    EXPECT_TRUE(result.empty());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
