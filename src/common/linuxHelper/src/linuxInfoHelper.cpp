#include "linuxInfoHelper.hpp"

#include <cstdint>
#include <string>
#include <unistd.h>

#include "file_io_utils.hpp"

namespace Utils
{
    uint64_t getBootTime()
    {
        static uint64_t btime;

        try
        {
            if (0UL == btime)
            {
                const std::string key {"btime "};
                const auto fileIoWrapper = std::make_unique<file_io::FileIOUtils>();
                const auto file {fileIoWrapper->getFileContent("/proc/stat")};

                btime = std::stoull(file.substr(file.find(key) + key.length()));
            }
        }
        catch (...) // NOLINT (bugprone-empty-catch)
        {
        }

        return btime;
    }

    uint64_t getClockTick()
    {
        const static uint64_t tick {static_cast<uint64_t>(sysconf(_SC_CLK_TCK))};

        return tick;
    }

    uint64_t timeTick2unixTime(const uint64_t startTime)
    {
        return (startTime / getClockTick()) + getBootTime();
    }
} // namespace Utils
