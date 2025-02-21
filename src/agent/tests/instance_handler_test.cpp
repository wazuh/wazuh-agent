#include <gtest/gtest.h>

#include <instance_handler.hpp>

#include <filesystem>
#include <fstream>

class InstanceHandlerTest : public ::testing::Test
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

std::filesystem::path InstanceHandlerTest::m_tempConfigFilePath;

TEST_F(InstanceHandlerTest, GetInstanceLock)
{
    auto instanceHandler = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml");
    const bool res = instanceHandler.isLockAcquired();
    ASSERT_TRUE(res);
}

TEST_F(InstanceHandlerTest, GetInstanceLockTwice)
{
    auto instanceHandler = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml");
    auto instanceHandler2 = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml");

    const bool resinstanceHandler = instanceHandler.isLockAcquired();
    const bool resinstanceHandler2 = instanceHandler2.isLockAcquired();

    ASSERT_TRUE(resinstanceHandler);
    ASSERT_FALSE(resinstanceHandler2);
}

TEST_F(InstanceHandlerTest, GetAgentStatusRunning)
{
    auto instanceHandler = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml");
    const std::string res = instance_handler::GetAgentStatus("./temp_wazuh-agent.yml");
    ASSERT_EQ(res, "running");
}

TEST_F(InstanceHandlerTest, GetAgentStatusStoppedPrevInstanceDestroyed)
{
    {
        auto instanceHandler = instance_handler::GetInstanceHandler("./temp_wazuh-agent.yml");
    }
    const std::string res = instance_handler::GetAgentStatus("./temp_wazuh-agent.yml");
    ASSERT_EQ(res, "stopped");
}

TEST_F(InstanceHandlerTest, GetAgentStatusStoppedNoPrevInstance)
{
    const std::string res = instance_handler::GetAgentStatus("./temp_wazuh-agent.yml");
    ASSERT_EQ(res, "stopped");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
