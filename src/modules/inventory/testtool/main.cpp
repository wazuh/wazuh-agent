/*
 * Wazuh Inventory Test tool
 * Copyright (C) 2015, Wazuh Inc.
 * October 7, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <memory>
#include <chrono>
#include "defs.h"
#include "configuration.hpp"
#include "inventory.hpp"

constexpr int DEFAULT_SLEEP_TIME { 60 };

int main(int argc, const char* argv[])
{
    auto timedMainLoop { false };
    auto sleepTime { DEFAULT_SLEEP_TIME };
    Configuration config{};

    if (2 == argc)
    {
        timedMainLoop = true;
        std::string firstArgument { argv[1] };

        sleepTime = firstArgument.find_first_not_of("0123456789") == std::string::npos ? std::stoi(firstArgument) : DEFAULT_SLEEP_TIME;

    }
    else if (2 < argc)
    {
        return -1;
    }

    Inventory::instance().setup(config);

    try
    {
        std::thread thread
        {
            [timedMainLoop, sleepTime]
            {
                if (!timedMainLoop)
                {
                    while (std::cin.get() != 'q');
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
                }

                Inventory::instance().stop();
            }
        };

        Inventory::instance().start();

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
