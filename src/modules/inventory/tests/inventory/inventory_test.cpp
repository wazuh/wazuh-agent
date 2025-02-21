#include "inventory.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class InventoryTest : public ::testing::Test
{
protected:
    void SetUp() override {}

    void TearDown() override {}

    Inventory& inventory = Inventory::Instance();
};

TEST_F(InventoryTest, SendUpdateEvent_Stateful)
{
    ::testing::MockFunction<int(const Message&)> mockPushMessage;
    inventory.SetPushMessageFunction(mockPushMessage.AsStdFunction());

    EXPECT_CALL(mockPushMessage, Call(::testing::_))
        .WillOnce(
            [](const Message& msg)
            {
                EXPECT_EQ(msg.type, MessageType::STATEFUL);
                const nlohmann::json expectedData = {{"key", "value"}};
                const nlohmann::json expectedMetadata = {
                    {"collector", "hardware"}, {"operation", "update"}, {"id", "123"}};
                EXPECT_EQ(msg.data, expectedData);
                EXPECT_EQ(nlohmann::json::parse(msg.metaData), expectedMetadata);
                return 1;
            });

    auto inputData = R"({
        "metadata": {
            "collector": "hardware",
            "operation": "update",
            "id": "123"
        },
        "data": {"key": "value"}
    })";

    inventory.SendDeltaEvent(inputData);
}

TEST_F(InventoryTest, SendDeleteEvent_Stateful)
{
    ::testing::MockFunction<int(const Message&)> mockPushMessage;
    inventory.SetPushMessageFunction(mockPushMessage.AsStdFunction());

    EXPECT_CALL(mockPushMessage, Call(::testing::_))
        .WillOnce(
            [](const Message& msg)
            {
                EXPECT_EQ(msg.type, MessageType::STATEFUL);
                const auto expectedData = nlohmann::json::object();
                const nlohmann::json expectedMetadata = {
                    {"collector", "hardware"}, {"operation", "delete"}, {"id", "123"}};
                EXPECT_EQ(msg.data, expectedData);
                EXPECT_EQ(nlohmann::json::parse(msg.metaData), expectedMetadata);
                return 1;
            });

    auto inputData = R"({
        "metadata": {
            "collector": "hardware",
            "operation": "delete",
            "id": "123"
        },
        "data": {"key": "value"}
    })";

    inventory.SendDeltaEvent(inputData);
}

TEST_F(InventoryTest, SendUpdateEvent_WithStateless)
{
    ::testing::MockFunction<int(const Message&)> mockPushMessage;
    inventory.SetPushMessageFunction(mockPushMessage.AsStdFunction());

    EXPECT_CALL(mockPushMessage, Call(::testing::_))
        .Times(2)
        .WillOnce(
            [](const Message& msg)
            {
                EXPECT_EQ(msg.type, MessageType::STATEFUL);
                const nlohmann::json expectedData = {{"key", "value"}};
                EXPECT_EQ(msg.data, expectedData);
                return 1;
            })
        .WillOnce(
            [](const Message& msg)
            {
                EXPECT_EQ(msg.type, MessageType::STATELESS);
                const nlohmann::json expectedData = {{"alert", "high"}};
                EXPECT_EQ(msg.data, expectedData);
                return 1;
            });

    auto inputData = R"({
        "metadata": {
            "collector": "hardware",
            "operation": "update",
            "id": "123"
        },
        "data": {"key": "value"},
        "stateless": {"alert": "high"}
    })";

    inventory.SendDeltaEvent(inputData);
}

TEST_F(InventoryTest, PushMessageFails_LogsWarning)
{
    ::testing::MockFunction<int(const Message&)> mockPushMessage;
    inventory.SetPushMessageFunction(mockPushMessage.AsStdFunction());

    EXPECT_CALL(mockPushMessage, Call(::testing::_)).WillOnce([](const Message&) { return 0; });

    auto inputData = R"({
        "metadata": {
            "collector": "hardware",
            "operation": "update",
            "id": "123"
        },
        "data": {"key": "value"}
    })";

    inventory.SendDeltaEvent(inputData);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
