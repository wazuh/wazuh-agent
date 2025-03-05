#include <chrono>
#include <fstream>
#include <inventory.hpp>
#include <iostream>
#include <memory>
#include <stdio.h>

constexpr int DEFAULT_SLEEP_TIME {60};

int main(int argc, const char* argv[])
{
    auto timedMainLoop {false};
    auto sleepTime {DEFAULT_SLEEP_TIME};
    std::shared_ptr<const configuration::ConfigurationParser> configurationParser;

    if (2 == argc)
    {
        timedMainLoop = true;
        std::string firstArgument {argv[1]};

        sleepTime = firstArgument.find_first_not_of("0123456789") == std::string::npos ? std::stoi(firstArgument)
                                                                                       : DEFAULT_SLEEP_TIME;
    }
    else if (2 < argc)
    {
        return -1;
    }

    configurationParser = std::make_shared<configuration::ConfigurationParser>();
    Inventory::Instance().Setup(configurationParser);

    try
    {
        std::thread thread {[timedMainLoop, sleepTime]
                            {
                                if (!timedMainLoop)
                                {
                                    while (std::cin.get() != 'q');
                                }
                                else
                                {
                                    std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
                                }

                                Inventory::Instance().Stop();
                            }};

        Inventory::Instance().Start();

        if (thread.joinable())
        {
            thread.join();
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
