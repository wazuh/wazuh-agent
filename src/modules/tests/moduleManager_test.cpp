#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <imodule.hpp>
#include <moduleManager.hpp>

#include <memory>
#include <string>

// Mock classes to simulate modules
class MockModule : public IModule
{
public:
    MOCK_METHOD(void, Run, (), (override));
    MOCK_METHOD(void, Setup, (std::shared_ptr<const configuration::ConfigurationParser>), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(boost::asio::awaitable<module_command::CommandExecutionResult>,
                ExecuteCommand,
                (const std::string, const nlohmann::json),
                (override));
    MOCK_METHOD(const std::string&, Name, (), (const override));
    MOCK_METHOD(void, SetPushMessageFunction, (const std::function<int(Message)>&), (override));

    static const std::string m_mockModule;
    static const std::string m_mockModule1;
    static const std::string m_mockModule2;
};

const std::string MockModule::m_mockModule = "MockModule";
const std::string MockModule::m_mockModule1 = "MockModule1";
const std::string MockModule::m_mockModule2 = "MockModule2";

class ModuleManagerTest : public ::testing::Test
{
protected:
    std::function<int(Message)> pushMessage;
    std::shared_ptr<configuration::ConfigurationParser> configurationParser;
    std::unique_ptr<ModuleManager> manager;
    std::shared_ptr<MockModule> m_mockModule;

    ModuleManagerTest()
        : pushMessage([](const Message&) { return 0; })
        , configurationParser(std::make_shared<configuration::ConfigurationParser>())
    {
        m_mockModule = std::make_shared<MockModule>();
    }

    void SetUp() override
    {
        // Set up default expectations for mock methods
        ON_CALL(*m_mockModule, Name()).WillByDefault(testing::ReturnRef(MockModule::m_mockModule));

        manager = std::make_unique<ModuleManager>(pushMessage, configurationParser, "uuid1234");
        taskExecuted = false;
    }

    std::atomic<bool> taskExecuted;
    std::mutex mtx;
    std::condition_variable cv;
};

TEST_F(ModuleManagerTest, Constructor)
{
    EXPECT_NO_THROW(ModuleManager(pushMessage, configurationParser, "uuid1234"));
}

TEST_F(ModuleManagerTest, AddModule)
{
    EXPECT_CALL(*m_mockModule, Name()).Times(1);

    manager->AddModule(m_mockModule);

    auto moduleWrapper = manager->GetModule("MockModule");
    EXPECT_NE(moduleWrapper, nullptr);
}

TEST_F(ModuleManagerTest, AddMultipleModules)
{
    auto m_mockModule1 = std::make_shared<MockModule>();
    auto m_mockModule2 = std::make_shared<MockModule>();

    EXPECT_CALL(*m_mockModule1, Name()).WillOnce(testing::ReturnRef(MockModule::m_mockModule1));
    EXPECT_CALL(*m_mockModule2, Name()).WillOnce(testing::ReturnRef(MockModule::m_mockModule2));

    manager->AddModule(m_mockModule1);
    manager->AddModule(m_mockModule2);

    auto moduleWrapper1 = manager->GetModule("MockModule1");
    auto moduleWrapper2 = manager->GetModule("MockModule2");

    EXPECT_NE(moduleWrapper1, nullptr);
    EXPECT_NE(moduleWrapper2, nullptr);
}

TEST_F(ModuleManagerTest, AddModuleDuplicateName)
{
    auto m_mockModule1 = std::make_shared<MockModule>();
    auto m_mockModule2 = std::make_shared<MockModule>();

    EXPECT_CALL(*m_mockModule1, Name()).WillOnce(testing::ReturnRef(MockModule::m_mockModule));
    EXPECT_CALL(*m_mockModule2, Name()).WillOnce(testing::ReturnRef(MockModule::m_mockModule));

    manager->AddModule(m_mockModule1);

    EXPECT_THROW(manager->AddModule(m_mockModule2), std::runtime_error);
}

TEST_F(ModuleManagerTest, GetModuleNotFound)
{
    auto moduleWrapper = manager->GetModule("NonExistentModule");
    EXPECT_EQ(moduleWrapper, nullptr);
}

TEST_F(ModuleManagerTest, SetupModules)
{
    EXPECT_CALL(*m_mockModule, Name()).Times(1);
    EXPECT_CALL(*m_mockModule, Setup(testing::_)).Times(1);

    manager->AddModule(m_mockModule);
    manager->Setup();
}

TEST_F(ModuleManagerTest, SetupMultipleModules)
{
    auto m_mockModule1 = std::make_shared<MockModule>();
    auto m_mockModule2 = std::make_shared<MockModule>();

    EXPECT_CALL(*m_mockModule1, Name()).WillOnce(testing::ReturnRef(MockModule::m_mockModule1));
    EXPECT_CALL(*m_mockModule2, Name()).WillOnce(testing::ReturnRef(MockModule::m_mockModule2));

    EXPECT_CALL(*m_mockModule1, Setup(testing::_)).Times(1);
    EXPECT_CALL(*m_mockModule2, Setup(testing::_)).Times(1);

    manager->AddModule(m_mockModule1);
    manager->AddModule(m_mockModule2);
    manager->Setup();
}

TEST_F(ModuleManagerTest, StartModules)
{
    EXPECT_CALL(*m_mockModule, Name()).Times(3);
    EXPECT_CALL(*m_mockModule, Run())
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs(
            [&]()
            {
                {
                    const std::lock_guard<std::mutex> lock(mtx);
                    taskExecuted = true;
                }
                cv.notify_one();
            }));

    manager->AddModule(m_mockModule);
    manager->Start();

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return taskExecuted.load(); });
    }

    auto moduleWrapper = manager->GetModule("MockModule");
    EXPECT_EQ(moduleWrapper->Name(), "MockModule");

    manager->Stop();
}

TEST_F(ModuleManagerTest, StartMultipleModules)
{
    auto m_mockModule1 = std::make_shared<MockModule>();
    auto m_mockModule2 = std::make_shared<MockModule>();

    EXPECT_CALL(*m_mockModule1, Name()).WillRepeatedly(testing::ReturnRef(MockModule::m_mockModule1));
    EXPECT_CALL(*m_mockModule2, Name()).WillRepeatedly(testing::ReturnRef(MockModule::m_mockModule2));

    EXPECT_CALL(*m_mockModule1, Run())
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs(
            [&]()
            {
                {
                    const std::lock_guard<std::mutex> lock(mtx);
                    taskExecuted = true;
                }
                cv.notify_one();
            }));
    EXPECT_CALL(*m_mockModule2, Run())
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs(
            [&]()
            {
                {
                    const std::lock_guard<std::mutex> lock(mtx);
                    taskExecuted = true;
                }
                cv.notify_one();
            }));
    EXPECT_CALL(*m_mockModule1, Stop()).Times(1);
    EXPECT_CALL(*m_mockModule2, Stop()).Times(1);

    manager->AddModule(m_mockModule1);
    manager->AddModule(m_mockModule2);

    manager->Start();

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return taskExecuted.load(); });
    }

    auto moduleWrapper1 = manager->GetModule("MockModule1");
    auto moduleWrapper2 = manager->GetModule("MockModule2");

    EXPECT_NE(moduleWrapper1, nullptr);
    EXPECT_NE(moduleWrapper2, nullptr);

    EXPECT_EQ(moduleWrapper1->Name(), "MockModule1");
    EXPECT_EQ(moduleWrapper2->Name(), "MockModule2");

    manager->Stop();
}

TEST_F(ModuleManagerTest, StopModules)
{
    EXPECT_CALL(*m_mockModule, Name()).Times(1);
    EXPECT_CALL(*m_mockModule, Stop()).Times(1);

    manager->AddModule(m_mockModule);
    manager->Stop();
}

TEST_F(ModuleManagerTest, StopMultipleModules)
{
    auto m_mockModule1 = std::make_shared<MockModule>();
    auto m_mockModule2 = std::make_shared<MockModule>();

    EXPECT_CALL(*m_mockModule1, Name()).WillOnce(testing::ReturnRef(MockModule::m_mockModule1));
    EXPECT_CALL(*m_mockModule2, Name()).WillOnce(testing::ReturnRef(MockModule::m_mockModule2));

    EXPECT_CALL(*m_mockModule1, Stop()).Times(1);
    EXPECT_CALL(*m_mockModule2, Stop()).Times(1);

    manager->AddModule(m_mockModule1);
    manager->AddModule(m_mockModule2);
    manager->Stop();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
