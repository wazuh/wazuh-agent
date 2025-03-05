#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pal.h>
#include <time.h>

class PalTimeTest : public ::testing::Test
{
protected:
    struct tm result
    {
    };

    time_t now = time(nullptr);
};

TEST_F(PalTimeTest, LocaltimeRSuccess)
{
    struct tm expectedResult;
    errno_t err = localtime_s(&expectedResult, &now);
    ASSERT_EQ(err, 0);

    struct tm* ret = localtime_r(&now, &result);
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ret, &result);

    EXPECT_EQ(result.tm_year, expectedResult.tm_year);
    EXPECT_EQ(result.tm_mon, expectedResult.tm_mon);
    EXPECT_EQ(result.tm_mday, expectedResult.tm_mday);
}

TEST_F(PalTimeTest, LocaltimeRNullInput)
{
    struct tm* ret = localtime_r(nullptr, &result);
    EXPECT_EQ(ret, nullptr);
}

TEST_F(PalTimeTest, LocaltimeRNullOutput)
{
    struct tm* ret = localtime_r(&now, nullptr);
    EXPECT_EQ(ret, nullptr);
}

TEST_F(PalTimeTest, GmtimeRSuccess)
{
    struct tm expectedResult;
    errno_t err = gmtime_s(&expectedResult, &now);
    ASSERT_EQ(err, 0);

    struct tm* ret = gmtime_r(&now, &result);
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ret, &result);

    EXPECT_EQ(result.tm_year, expectedResult.tm_year);
    EXPECT_EQ(result.tm_mon, expectedResult.tm_mon);
    EXPECT_EQ(result.tm_mday, expectedResult.tm_mday);
}

TEST_F(PalTimeTest, GmtimeRNullInput)
{
    struct tm* ret = gmtime_r(nullptr, &result);
    EXPECT_EQ(ret, nullptr);
}

TEST_F(PalTimeTest, GmtimeRNullOutput)
{
    struct tm* ret = gmtime_r(&now, nullptr);
    EXPECT_EQ(ret, nullptr);
}
