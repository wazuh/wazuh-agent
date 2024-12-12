#include <cstdio>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "inventory.hpp"

constexpr auto INVENTORY_DB_PATH {"TEMP.db"};

class InventoryTest : public ::testing::Test
{
    protected:

        void SetUp() override {}
        void TearDown() override
        {
            std::remove(INVENTORY_DB_PATH);
        }

        Inventory &inventory = Inventory::Instance();
};

TEST_F(InventoryTest, SendUpdateEvent) {
    ::testing::MockFunction<int(const Message&)> mockPushMessage;

    inventory.SetPushMessageFunction(mockPushMessage.AsStdFunction());

    EXPECT_CALL(mockPushMessage, Call(::testing::_))
        .WillOnce([](const Message& msg)
        {
            auto expectedData = R"({"key":"value"})";
            auto expectedMetadata = R"({"id":"123","module":"inventory","operation":"update","type":"hardware"})";

            EXPECT_EQ(msg.data.dump(), expectedData);
            EXPECT_EQ(msg.metaData, expectedMetadata);
            return 1;
        });

    auto inputData = R"({
        "type": "hardware",
        "operation": "update",
        "id": "123",
        "data": {"key": "value"}
    })";

    inventory.SendDeltaEvent(inputData);
}

TEST_F(InventoryTest, SendDeleteEvent) {
    ::testing::MockFunction<int(const Message&)> mockPushMessage;

    inventory.SetPushMessageFunction(mockPushMessage.AsStdFunction());

    EXPECT_CALL(mockPushMessage, Call(::testing::_))
        .WillOnce([](const Message& msg)
        {
            auto expectedData = R"({})";
            auto expectedMetadata = R"({"id":"123","module":"inventory","operation":"delete","type":"hardware"})";

            EXPECT_EQ(msg.data.dump(), expectedData);
            EXPECT_EQ(msg.metaData, expectedMetadata);
            return 1;
        });

    auto inputData = R"({
        "type": "hardware",
        "operation": "delete",
        "id": "123",
        "data": {"key": "value"}
    })";

    inventory.SendDeltaEvent(inputData);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
