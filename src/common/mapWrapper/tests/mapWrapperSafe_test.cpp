#include "mapWrapperSafe_test.hpp"
#include "mapWrapperSafe.hpp"
#include <chrono>
#include <thread>

void MapWrapperSafeTest::SetUp() {};

void MapWrapperSafeTest::TearDown() {};

TEST_F(MapWrapperSafeTest, insertTest)
{
    Utils::MapWrapperSafe<int, int> mapSafe;
    mapSafe.insert(1, 2);
    EXPECT_EQ(2, mapSafe[1]);
}

TEST_F(MapWrapperSafeTest, eraseTest)
{
    Utils::MapWrapperSafe<int, int> mapSafe;
    mapSafe.insert(1, 2);
    EXPECT_NO_THROW(mapSafe.erase(1));
    EXPECT_EQ(0, mapSafe[1]);
}
