#pragma once

#include <chrono>
#include <iostream>

class TimeMeasurement
{
public:
    TimeMeasurement(const char* function, bool bMicro = false)
        : mFunction(function)
        , mMicro(bMicro)
    {
        mStart = std::chrono::high_resolution_clock::now();
    }

    ~TimeMeasurement()
    {
        char sz[256];
        auto finish = std::chrono::high_resolution_clock::now();
        if(mMicro)
        {
            std::chrono::duration<double, std::micro> elapsed = finish - mStart;
            sprintf(sz, "%s: %d micro\n", mFunction, static_cast<int>(elapsed.count()));
        }
        else
        {
            std::chrono::duration<double, std::milli> elapsed = finish - mStart;
            sprintf(sz, "%s: %d ms\n", mFunction, static_cast<int>(elapsed.count()));
        }

        if (!bSilent)
            std::cout << sz;
    }

    double GetElapsedMs()
    {
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = finish - mStart;
        return elapsed.count();
    }

private:
    std::chrono::_V2::system_clock::time_point mStart;
    const char* mFunction; // message
    bool mMicro{false};
    bool bSilent{true};
};