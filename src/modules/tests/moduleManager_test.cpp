#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <moduleManager.hpp>

#include <memory>

// Mock classes to simulate modules
class MockModule
{
public:
    MOCK_METHOD(void, Start, (), ());
    MOCK_METHOD(void, Setup, (std::shared_ptr<const configuration::ConfigurationParser>), ());
    MOCK_METHOD(void, Stop, (), ());
    MOCK_METHOD(boost::asio::awaitable<module_command::CommandExecutionResult>,
                ExecuteCommand,
                (const std::string&, const nlohmann::json&),
                ());
    MOCK_METHOD(std::string, Name, (), (const));
    MOCK_METHOD(void, SetPushMessageFunction, (const std::function<int(Message)>));
};

class ModuleManagerTest : public ::testing::Test
{
protected:
    std::function<int(Message)> pushMessage;
    std::shared_ptr<configuration::ConfigurationParser> configurationParser;
    std::unique_ptr<ModuleManager> manager;
    MockModule mockModule;

    ModuleManagerTest()
        : pushMessage([](const Message&) { return 0; })
        , configurationParser(std::make_shared<configuration::ConfigurationParser>())
    {
    }

    void SetUp() override
    {
        // Set up default expectations for mock methods
        ON_CALL(mockModule, Name()).WillByDefault(testing::Return("MockModule"));

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
    EXPECT_CALL(mockModule, Name()).Times(1);

    manager->AddModule(mockModule);

    auto moduleWrapper = manager->GetModule("MockModule");
    EXPECT_NE(moduleWrapper, nullptr);
}

TEST_F(ModuleManagerTest, AddMultipleModules)
{
    MockModule mockModule1, mockModule2;

    EXPECT_CALL(mockModule1, Name()).WillOnce(testing::Return("MockModule1"));
    EXPECT_CALL(mockModule2, Name()).WillOnce(testing::Return("MockModule2"));

    manager->AddModule(mockModule1);
    manager->AddModule(mockModule2);

    auto moduleWrapper1 = manager->GetModule("MockModule1");
    auto moduleWrapper2 = manager->GetModule("MockModule2");

    EXPECT_NE(moduleWrapper1, nullptr);
    EXPECT_NE(moduleWrapper2, nullptr);
}

TEST_F(ModuleManagerTest, AddModuleDuplicateName)
{
    MockModule mockModule1, mockModule2;

    EXPECT_CALL(mockModule1, Name()).WillOnce(testing::Return("MockModule"));
    EXPECT_CALL(mockModule2, Name()).WillOnce(testing::Return("MockModule"));

    manager->AddModule(mockModule1);

    EXPECT_THROW(manager->AddModule(mockModule2), std::runtime_error);
}

TEST_F(ModuleManagerTest, GetModuleNotFound)
{
    auto moduleWrapper = manager->GetModule("NonExistentModule");
    EXPECT_EQ(moduleWrapper, nullptr);
}

TEST_F(ModuleManagerTest, SetupModules)
{
    EXPECT_CALL(mockModule, Name()).Times(1);
    EXPECT_CALL(mockModule, Setup(testing::_)).Times(1);

    manager->AddModule(mockModule);
    manager->Setup();
}

TEST_F(ModuleManagerTest, SetupMultipleModules)
{
    MockModule mockModule1, mockModule2;

    EXPECT_CALL(mockModule1, Name()).WillOnce(testing::Return("MockModule1"));
    EXPECT_CALL(mockModule2, Name()).WillOnce(testing::Return("MockModule2"));

    EXPECT_CALL(mockModule1, Setup(testing::_)).Times(1);
    EXPECT_CALL(mockModule2, Setup(testing::_)).Times(1);

    manager->AddModule(mockModule1);
    manager->AddModule(mockModule2);
    manager->Setup();
}

TEST_F(ModuleManagerTest, StartModules)
{
    EXPECT_CALL(mockModule, Name()).Times(3);
    EXPECT_CALL(mockModule, Start())
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

    manager->AddModule(mockModule);
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
    MockModule mockModule1, mockModule2;

    EXPECT_CALL(mockModule1, Name()).Times(3).WillRepeatedly(testing::Return("MockModule1"));
    EXPECT_CALL(mockModule2, Name()).Times(3).WillRepeatedly(testing::Return("MockModule2"));

    EXPECT_CALL(mockModule1, Start())
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
    EXPECT_CALL(mockModule2, Start())
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
    EXPECT_CALL(mockModule1, Stop()).Times(1);
    EXPECT_CALL(mockModule2, Stop()).Times(1);

    manager->AddModule(mockModule1);
    manager->AddModule(mockModule2);

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
    EXPECT_CALL(mockModule, Name()).Times(1);
    EXPECT_CALL(mockModule, Stop()).Times(1);

    manager->AddModule(mockModule);
    manager->Stop();
}

TEST_F(ModuleManagerTest, StopMultipleModules)
{
    MockModule mockModule1, mockModule2;

    EXPECT_CALL(mockModule1, Name()).WillOnce(testing::Return("MockModule1"));
    EXPECT_CALL(mockModule2, Name()).WillOnce(testing::Return("MockModule2"));

    EXPECT_CALL(mockModule1, Stop()).Times(1);
    EXPECT_CALL(mockModule2, Stop()).Times(1);

    manager->AddModule(mockModule1);
    manager->AddModule(mockModule2);
    manager->Stop();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
