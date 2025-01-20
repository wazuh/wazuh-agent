#include <gtest/gtest.h>

#include <oslogstore.hpp>

#include <chrono>

TEST(OSLogStoreWrapperTest, Constructor)
{
    [[maybe_unused]] OSLogStoreWrapper oslogstore;
    SUCCEED();
}

TEST(OSLogStoreWrapperTest, AllEntriesDoesNotThrow)
{
    OSLogStoreWrapper oslogstore;
    EXPECT_NO_THROW(
        oslogstore.AllEntries(static_cast<double>(std::chrono::system_clock::now().time_since_epoch().count()),
                              "process != \"wazuh-agent\"",
                              OSLogStoreWrapper::LogLevel::Info));
}

TEST(OSLogStoreWrapperTest, AllEntiresThrowForInvalidStartTimes)
{
    OSLogStoreWrapper oslogstore;
    EXPECT_ANY_THROW(oslogstore.AllEntries(0, "process != \"wazuh-agent\"", OSLogStoreWrapper::LogLevel::Info));
    EXPECT_ANY_THROW(oslogstore.AllEntries(-1, "process != \"wazuh-agent\"", OSLogStoreWrapper::LogLevel::Info));
}

TEST(OSLogStoreWrapperTest, AllEntriesThrowsWhenGivenAnInvalidQuery)
{
    OSLogStoreWrapper oslogstore;
    EXPECT_ANY_THROW(
        oslogstore.AllEntries(static_cast<double>(std::chrono::system_clock::now().time_since_epoch().count()),
                              "invalidfieldthatwillthrow != \"wazuh-agent\"",
                              OSLogStoreWrapper::LogLevel::Info));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
