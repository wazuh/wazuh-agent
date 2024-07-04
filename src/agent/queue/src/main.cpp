#include <iostream>

#include "queue.hpp"

int main()
{
    MultiTypeQueue queue(3); // with 2 it get's blocked

    // Push events
    queue.push(Message(STATE_LESS, R"({"Data" : "for STATE_LESS"})"));
    queue.push(Message(STATE_LESS, R"({"Data" : "for STATE_LESS"})"));
    queue.push(Message(STATE_LESS, R"({"Data" : "for STATE_LESS"})"));
    queue.push(Message(COMMAND, R"({"Data" : "for COMMAND"})"));

    // Retrieve events
    Message event1 = queue.getLastMessage(MessageType::STATE_LESS);
    std::cout << "Retrieved event 1: " << event1.data << std::endl;

    Message event2 = queue.getLastMessage(MessageType::STATE_LESS);
    std::cout << "Retrieved event 2: " << event2.data << std::endl;

    Message event3 = queue.getLastMessage(MessageType::STATE_LESS);
    std::cout << "Retrieved event 3: " << event3.data << std::endl;

    // Check if a specific queue is empty
    bool isEmptyType1 = queue.isEmptyByType(MessageType::STATE_FULL);
    std::cout << "Is STATE_LESS queue empty? " << (isEmptyType1 ? "Yes" : "No") << std::endl;

    return 0;
}