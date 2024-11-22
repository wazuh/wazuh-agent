#include <gtest/gtest.h>

#include <unix_daemon.hpp>

class UnixDaemonTest : public ::testing::Test
{
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(UnixDaemonTest, CreateLockFile)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile();
    bool res = lockFileHandler.isLockFileCreated();
    ASSERT_TRUE(res);
    lockFileHandler.removeLockFile();
}

TEST_F(UnixDaemonTest, CreateLockFileTwice)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile();
    unix_daemon::LockFileHandler lockFileHandler2 = unix_daemon::GenerateLockFile();

    bool reslockFileHandler = lockFileHandler.isLockFileCreated();
    bool reslockFileHandler2 = lockFileHandler2.isLockFileCreated();

    ASSERT_TRUE(reslockFileHandler);
    ASSERT_FALSE(reslockFileHandler2);
    lockFileHandler.removeLockFile();
}

TEST_F(UnixDaemonTest, GetDaemonStatusRunning)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile();
    std::string res = unix_daemon::GetDaemonStatus();
    ASSERT_EQ(res, "running");
    lockFileHandler.removeLockFile();
}

TEST_F(UnixDaemonTest, GetDaemonStatusStopped)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile();
    lockFileHandler.removeLockFile();
    std::string res = unix_daemon::GetDaemonStatus();
    ASSERT_EQ(res, "stopped");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
