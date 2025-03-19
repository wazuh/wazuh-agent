#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <instance_handler.hpp>
#include <mock_filesystem_wrapper.hpp>

#include <filesystem>
#include <fstream>

class InstanceHandlerTest : public ::testing::Test
{
protected:
    static std::filesystem::path m_tempConfigFilePath;
    std::shared_ptr<MockFileSystemWrapper> m_fileSystemWrapper;

    void SetUp() override
    {
        m_fileSystemWrapper = std::make_shared<MockFileSystemWrapper>();
    }

    void TearDown() override
    {
        m_fileSystemWrapper.reset();
    }

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

std::filesystem::path InstanceHandlerTest::m_tempConfigFilePath;

TEST_F(InstanceHandlerTest, GetInstanceLock)
{
    EXPECT_CALL(*m_fileSystemWrapper, exists(testing::_)).WillOnce(::testing::Return(false));
    EXPECT_CALL(*m_fileSystemWrapper, create_directories(testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, open(testing::_, testing::_, testing::_)).WillOnce(::testing::Return(1));
    EXPECT_CALL(*m_fileSystemWrapper, flock(testing::_, testing::_)).WillOnce(::testing::Return(1));
    EXPECT_CALL(*m_fileSystemWrapper, remove(testing::_)).WillOnce(::testing::Return(1));

    auto instanceHandler = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml", m_fileSystemWrapper);
    const bool res = instanceHandler.isLockAcquired();
    ASSERT_TRUE(res);
}

TEST_F(InstanceHandlerTest, GetInstanceLockTwice)
{
    EXPECT_CALL(*m_fileSystemWrapper, exists(testing::_))
        .WillOnce(::testing::Return(false))
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, create_directories(testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, open(testing::_, testing::_, testing::_)).WillRepeatedly(::testing::Return(1));
    EXPECT_CALL(*m_fileSystemWrapper, flock(testing::_, testing::_))
        .WillOnce(::testing::Return(1))   // the first time flock succeeds
        .WillOnce(::testing::Return(-1)); // the second time flock fails

    EXPECT_CALL(*m_fileSystemWrapper, is_directory(testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, close);
    EXPECT_CALL(*m_fileSystemWrapper, remove(testing::_)).WillOnce(::testing::Return(1));

    auto instanceHandler = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml", m_fileSystemWrapper);
    auto instanceHandler2 = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml", m_fileSystemWrapper);

    const bool retInstanceHandler = instanceHandler.isLockAcquired();
    const bool retInstanceHandler2 = instanceHandler2.isLockAcquired();

    ASSERT_TRUE(retInstanceHandler);
    ASSERT_FALSE(retInstanceHandler2);
}

TEST_F(InstanceHandlerTest, GetAgentStatusRunning)
{
    EXPECT_CALL(*m_fileSystemWrapper, exists(testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, create_directories(testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, open(testing::_, testing::_, testing::_)).WillRepeatedly(::testing::Return(1));
    EXPECT_CALL(*m_fileSystemWrapper, flock(testing::_, testing::_))
        .WillOnce(::testing::Return(1)) // the first time flock() succeeds
        .WillOnce(::testing::Invoke(
            [](int, int)
            {
                errno = EAGAIN;
                return -1;
            })); // the second time flock() fails and sets errno as needed in GetAgentStatus()
    EXPECT_CALL(*m_fileSystemWrapper, is_directory(testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, close);
    EXPECT_CALL(*m_fileSystemWrapper, remove(testing::_)).WillRepeatedly(::testing::Return(1));

    auto instanceHandler = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml", m_fileSystemWrapper);
    const bool retInstanceHandler = instanceHandler.isLockAcquired();
    ASSERT_TRUE(retInstanceHandler);

    const std::string res = instance_handler::GetAgentStatus("./temp_wazuh-agent.yml", m_fileSystemWrapper);
    ASSERT_EQ(res, "running");
}

TEST_F(InstanceHandlerTest, GetAgentStatusStoppedPrevInstanceDestroyed)
{
    EXPECT_CALL(*m_fileSystemWrapper, exists(testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, create_directories(testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, open(testing::_, testing::_, testing::_)).WillRepeatedly(::testing::Return(1));
    EXPECT_CALL(*m_fileSystemWrapper, flock(testing::_, testing::_)).WillRepeatedly(::testing::Return(1));
    EXPECT_CALL(*m_fileSystemWrapper, is_directory(testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, remove(testing::_)).WillRepeatedly(::testing::Return(1));

    {
        auto instanceHandler = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml", m_fileSystemWrapper);
    }
    const std::string res = instance_handler::GetAgentStatus("./temp_wazuh-agent.yml", m_fileSystemWrapper);
    ASSERT_EQ(res, "stopped");
}

TEST_F(InstanceHandlerTest, GetAgentStatusStoppedNoPrevInstance)
{
    EXPECT_CALL(*m_fileSystemWrapper, exists(testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, create_directories(testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, open(testing::_, testing::_, testing::_)).WillRepeatedly(::testing::Return(1));
    EXPECT_CALL(*m_fileSystemWrapper, flock(testing::_, testing::_)).WillRepeatedly(::testing::Return(1));
    EXPECT_CALL(*m_fileSystemWrapper, is_directory(testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*m_fileSystemWrapper, remove(testing::_)).WillRepeatedly(::testing::Return(1));

    const std::string res = instance_handler::GetAgentStatus("./temp_wazuh-agent.yml");
    ASSERT_EQ(res, "stopped");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
