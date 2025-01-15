#include <gtest/gtest.h>

#include <oslogstore_wrapper.hpp>

#include <chrono>

TEST(OSLogStoreWrapperTest, start)
{
    OSLogStoreWrapper oslogstore;
    EXPECT_NO_THROW(oslogstore.AllEntries(std::chrono::system_clock::now().time_since_epoch().count(),
                                          "process != \"wazuh-agent\"",
                                          OSLogStoreWrapper::LogLevel::Info));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
