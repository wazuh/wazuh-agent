#include <gtest/gtest.h>
#include "../include/queue.hpp"
// #include "../include/shared.hpp"

// Example test case
TEST(test_queue, Base) {
    // EXPECT_EQ(result.z, 9);
    MultiTypeQueue queue(10); // with 2 it get's blocked
    int items_to_insert = 5;
    auto dataContent = R"({"Data" : "for STATE_LESS)" + std::to_string(0) + R"("})";
    queue.push(Message(MessageType::STATE_LESS, dataContent));
    auto messageResponse = queue.getLastMessage(MessageType::STATE_LESS);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
