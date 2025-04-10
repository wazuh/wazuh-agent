#include <chrono>
#include <cstdio>
#include <fstream>
#include <inventory.hpp>
#include <iostream>
#include <memory>

constexpr int DEFAULT_SLEEP_TIME {60};

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char* argv[])
{
    auto timedMainLoop {false};
    auto sleepTime {DEFAULT_SLEEP_TIME};
    std::shared_ptr<const configuration::ConfigurationParser> configurationParser;

    if (2 == argc)
    {
        timedMainLoop = true;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const std::string firstArgument {argv[1]};

        sleepTime = firstArgument.find_first_not_of("0123456789") == std::string::npos ? std::stoi(firstArgument)
                                                                                       : DEFAULT_SLEEP_TIME;
    }
    else if (2 < argc)
    {
        return -1;
    }

    configurationParser = std::make_shared<configuration::ConfigurationParser>();
    auto inventory = std::make_shared<Inventory>();
    inventory->Setup(configurationParser);

    try
    {
        std::thread thread {[timedMainLoop, sleepTime, inventory]
                            {
                                if (!timedMainLoop)
                                {
                                    while (std::cin.get() != 'q');
                                }
                                else
                                {
                                    std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
                                }

                                inventory->Stop();
                            }};

        inventory->Run();

        if (thread.joinable())
        {
            thread.join();
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << '\n';
    }

    return 0;
}
