#include <gtest/gtest.h>

#include <unix_daemon.hpp>

#include <string>

class UnixDaemonTest : public ::testing::Test
{
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(UnixDaemonTest, CreateAndReadPIDFile)
{
    unix_daemon::PIDFileHandler pidHandler;

    pid_t pid = unix_daemon::PIDFileHandler::ReadPIDFromFile();
    ASSERT_EQ(getpid(), pid);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
