#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace file_io
{
    class FileIO
    {
        public:
            void readLineByLine(const std::filesystem::path& filePath,
            const std::function<bool(const std::string&)>& callback);
    };
}
