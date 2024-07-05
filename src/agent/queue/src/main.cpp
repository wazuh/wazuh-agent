#include <iostream>
#include <string>
#include <thread>

#include "queue.hpp"

int main()
{
    MultiTypeQueue queue(10); // with 2 it get's blocked

    auto consumer1 = [&](int &count) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        int item;
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::STATE_LESS);
            std::cout << "Popping event 1: " << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    auto consumer2 = [&](int &count) {
        int item;
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::STATE_FULL);
            std::cout << "Popping event 2: " << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };


    auto producer = [&](int &count) {
        for (int i = 0; i < count; ++i)
        {
            auto message = R"({"Data" : "for STATE_LESS)" + std::to_string(i) + R"("})";
            queue.push(Message(MessageType::STATE_LESS, message));
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
        }
    };

    int items_to_insert = 5;
    int items_to_consume = 2;

    producer(items_to_insert);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::thread consumer_thread1(consumer1, std::ref(items_to_consume));
    std::thread producer_thread1(consumer2, std::ref(items_to_consume));

    producer_thread1.join();
    consumer_thread1.join();

    return 0;
}
