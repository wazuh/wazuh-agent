#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "moduleManager.hpp"

using namespace testing;

// Mock classes to simulate modules
class MockModule {
public:
    MOCK_METHOD(void, start, (), ());
    MOCK_METHOD(int, setup, (const Configuration&), ());
    MOCK_METHOD(void, stop, (), ());
    MOCK_METHOD(std::string, command, (const std::string&), ());
    MOCK_METHOD(std::string, name, (), (const));
};

class ModuleManagerTest : public ::testing::Test {
protected:
    ModuleManager manager;
    MockModule mockModule;

    void SetUp() override {
        // Set up default expectations for mock methods
        ON_CALL(mockModule, name()).WillByDefault(Return("MockModule"));
    }
};

TEST_F(ModuleManagerTest, AddModule) {
    EXPECT_CALL(mockModule, name()).Times(1);

    manager.addModule(mockModule);

    auto moduleWrapper = manager.getModule("MockModule");
    EXPECT_NE(moduleWrapper, nullptr);
}

TEST_F(ModuleManagerTest, AddMultipleModules) {
    MockModule mockModule1, mockModule2;

    EXPECT_CALL(mockModule1, name()).WillOnce(Return("MockModule1"));
    EXPECT_CALL(mockModule2, name()).WillOnce(Return("MockModule2"));

    manager.addModule(mockModule1);
    manager.addModule(mockModule2);

    auto moduleWrapper1 = manager.getModule("MockModule1");
    auto moduleWrapper2 = manager.getModule("MockModule2");

    EXPECT_NE(moduleWrapper1, nullptr);
    EXPECT_NE(moduleWrapper2, nullptr);
}

TEST_F(ModuleManagerTest, AddModuleDuplicateName) {
    MockModule mockModule1, mockModule2;

    EXPECT_CALL(mockModule1, name()).WillOnce(Return("MockModule"));
    EXPECT_CALL(mockModule2, name()).WillOnce(Return("MockModule"));

    manager.addModule(mockModule1);

    EXPECT_THROW(manager.addModule(mockModule2), std::runtime_error);
}

TEST_F(ModuleManagerTest, GetModuleNotFound) {
    auto moduleWrapper = manager.getModule("NonExistentModule");
    EXPECT_EQ(moduleWrapper, nullptr);
}

TEST_F(ModuleManagerTest, SetupModules) {
    Configuration config;
    EXPECT_CALL(mockModule, name()).Times(1);
    EXPECT_CALL(mockModule, setup(_)).Times(1);

    manager.addModule(mockModule);
    manager.setup(config);
}

TEST_F(ModuleManagerTest, SetupMultipleModules) {
    MockModule mockModule1, mockModule2;
    Configuration config;

    EXPECT_CALL(mockModule1, name()).WillOnce(Return("MockModule1"));
    EXPECT_CALL(mockModule2, name()).WillOnce(Return("MockModule2"));

    EXPECT_CALL(mockModule1, setup(_)).Times(1);
    EXPECT_CALL(mockModule2, setup(_)).Times(1);

    manager.addModule(mockModule1);
    manager.addModule(mockModule2);
    manager.setup(config);
}

TEST_F(ModuleManagerTest, SetupModuleThrowsException) {
    Configuration config;

    EXPECT_CALL(mockModule, name()).WillOnce(Return("MockModule"));
    EXPECT_CALL(mockModule, setup(_)).WillOnce(Throw(std::runtime_error("Setup failed")));

    manager.addModule(mockModule);

    EXPECT_THROW(manager.setup(config), std::runtime_error);
}

TEST_F(ModuleManagerTest, StartModules) {
    EXPECT_CALL(mockModule, name()).Times(2);
    EXPECT_CALL(mockModule, start()).Times(1);
    EXPECT_CALL(mockModule, stop()).Times(1);

    manager.addModule(mockModule);
    manager.start();

    auto moduleWrapper = manager.getModule("MockModule");
    EXPECT_EQ(moduleWrapper->name(), "MockModule");

    manager.stop();
}

TEST_F(ModuleManagerTest, StartMultipleModules) {
    MockModule mockModule1, mockModule2;

    EXPECT_CALL(mockModule1, name()).Times(2).WillRepeatedly(Return("MockModule1"));
    EXPECT_CALL(mockModule2, name()).Times(2).WillRepeatedly(Return("MockModule2"));

    EXPECT_CALL(mockModule1, start()).Times(1);
    EXPECT_CALL(mockModule2, start()).Times(1);
    EXPECT_CALL(mockModule1, stop()).Times(1);
    EXPECT_CALL(mockModule2, stop()).Times(1);

    manager.addModule(mockModule1);
    manager.addModule(mockModule2);

    manager.start();

    auto moduleWrapper1 = manager.getModule("MockModule1");
    auto moduleWrapper2 = manager.getModule("MockModule2");

    EXPECT_NE(moduleWrapper1, nullptr);
    EXPECT_NE(moduleWrapper2, nullptr);

    EXPECT_EQ(moduleWrapper1->name(), "MockModule1");
    EXPECT_EQ(moduleWrapper2->name(), "MockModule2");

    manager.stop();
}

TEST_F(ModuleManagerTest, StopModules) {
    EXPECT_CALL(mockModule, name()).Times(1);
    EXPECT_CALL(mockModule, stop()).Times(1);

    manager.addModule(mockModule);
    manager.stop();
}

TEST_F(ModuleManagerTest, StopMultipleModules) {
    MockModule mockModule1, mockModule2;

    EXPECT_CALL(mockModule1, name()).WillOnce(Return("MockModule1"));
    EXPECT_CALL(mockModule2, name()).WillOnce(Return("MockModule2"));

    EXPECT_CALL(mockModule1, stop()).Times(1);
    EXPECT_CALL(mockModule2, stop()).Times(1);

    manager.addModule(mockModule1);
    manager.addModule(mockModule2);
    manager.stop();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

