#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <oslogstore_wrapper.hpp>

TEST(OSLogStoreWrapperTest, start)
{
    OSLogStoreWrapper oslogstore;
    auto it = oslogstore.Begin("2025-01-03 17:20:00", "2025-01-03 17:21:01");
    while (it != oslogstore.End())
    {
        auto log = *it;
        std::cout << log << std::endl;
        ++it;
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
