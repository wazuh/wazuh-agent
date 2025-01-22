#include <gtest/gtest.h>

#include <instance_handler.hpp>

#include <filesystem>
#include <fstream>

class UnixDaemonTest : public ::testing::Test
{
protected:
    static std::filesystem::path m_tempConfigFilePath;

    static void SetUpTestSuite()
    {
        m_tempConfigFilePath = "temp_wazuh-agent.yml";

        std::ofstream outFile(m_tempConfigFilePath);
        outFile << R"(
            agent:
                path.run: "."
        )";
        outFile.close();
    }

    static void TearDownTestSuite()
    {
        std::filesystem::remove(m_tempConfigFilePath);
    }
};

std::filesystem::path UnixDaemonTest::m_tempConfigFilePath;

TEST_F(UnixDaemonTest, CreateLockFile)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile("./temp_wazuh-agent.yml");
    bool res = lockFileHandler.isLockFileCreated();
    ASSERT_TRUE(res);
}

TEST_F(UnixDaemonTest, CreateLockFileTwice)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile("./temp_wazuh-agent.yml");
    unix_daemon::LockFileHandler lockFileHandler2 = unix_daemon::GenerateLockFile("./temp_wazuh-agent.yml");

    bool reslockFileHandler = lockFileHandler.isLockFileCreated();
    bool reslockFileHandler2 = lockFileHandler2.isLockFileCreated();

    ASSERT_TRUE(reslockFileHandler);
    ASSERT_FALSE(reslockFileHandler2);
}

TEST_F(UnixDaemonTest, GetDaemonStatusRunning)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile("./temp_wazuh-agent.yml");
    std::string res = unix_daemon::GetDaemonStatus("./temp_wazuh-agent.yml");
    ASSERT_EQ(res, "running");
}

TEST_F(UnixDaemonTest, GetDaemonStatusStoppedPrevInstanceDestroyed)
{
    {
        unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile("./temp_wazuh-agent.yml");
    }
    std::string res = unix_daemon::GetDaemonStatus("./temp_wazuh-agent.yml");
    ASSERT_EQ(res, "stopped");
}

TEST_F(UnixDaemonTest, GetDaemonStatusStoppedNoPrevInstance)
{
    std::string res = unix_daemon::GetDaemonStatus("./temp_wazuh-agent.yml");
    ASSERT_EQ(res, "stopped");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
