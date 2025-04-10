#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <memory>

#include <instance_communicator.hpp>

using namespace testing;

class MockCallbacks
{
public:
    MOCK_METHOD(void, ReloadModules, (), ());
    MOCK_METHOD(void, ReloadModule, (const std::string& moduleName), ());
};

class InstanceCommunicatorTest : public Test
{
protected:
    void SetUp() override
    {
        // m_instanceCommunicator =
        //     std::make_unique<instance_communicator::InstanceCommunicator>([]() {}, [](const std::string&) {});
    }

    void TearDown() override {}

    MockCallbacks m_mockCallbacks;
    std::unique_ptr<instance_communicator::InstanceCommunicator> m_instanceCommunicator;
};

TEST(TestInstanceCommunicator, ConstructorTest)
{
    EXPECT_NO_THROW(const instance_communicator::InstanceCommunicator communicator([]() {}, [](const std::string&) {}));
    EXPECT_ANY_THROW(
        const instance_communicator::InstanceCommunicator communicator(nullptr, [](const std::string&) {}));
    EXPECT_ANY_THROW(const instance_communicator::InstanceCommunicator communicator([]() {}, nullptr));
}

TEST_F(InstanceCommunicatorTest, ReloadModuleHandlerIsCalled)
{
    // Set the expectation that mockDependency.ReloadModules() will be called exactly once.
    EXPECT_CALL(m_mockCallbacks, ReloadModules()).Times(::testing::Exactly(1));

    // Create an InstanceCommunicator instance, passing the mock method.
    const instance_communicator::InstanceCommunicator communicator(
        [this]() { m_mockCallbacks.ReloadModules(); },
        [](const std::string&) {
        } // Dummy lambda for the second handler if not relevant to this test
    );

    // Perform the action on the InstanceCommunicator that should trigger reloadModulesHandler.
    // For example, if there's a method like 'TriggerReloadAllModules':
    communicator.HandleSignal("RELOAD");
    // The EXPECT_CALL implicitly verifies if the expectation was met when the test ends.
}

TEST_F(InstanceCommunicatorTest, ReloadModulesHandlerIsCalled)
{
    // Define the expected module name.
    const std::string message = "RELOAD-MODULE:my_test_module";
    const std::string expectedModuleName = "my_test_module";

    // Set the expectation that mockDependency.ReloadModule() will be called exactly once
    // with the specific 'expectedModuleName'.
    EXPECT_CALL(m_mockCallbacks, ReloadModule(expectedModuleName)).Times(::testing::Exactly(1));

    // Create an InstanceCommunicator instance, passing the mock method.
    const instance_communicator::InstanceCommunicator communicator(
        []() {}, // Dummy lambda for the first handler if not relevant to this test
        [this](const std::string& moduleName) { m_mockCallbacks.ReloadModule(moduleName); });

    // Perform the action on the InstanceCommunicator that should trigger reloadModuleHandler
    // with the 'expectedModuleName'. For example, if there's a method like 'TriggerReloadModule':
    communicator.HandleSignal(message);

    // The EXPECT_CALL implicitly verifies if the expectation was met when the test ends.
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
