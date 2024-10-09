#pragma once

#include <vector>

#include "reader.hpp"

namespace logcollector {

class Localfile {
public:
    Localfile(std::string filePath);
    Awaitable run();

private:
    std::string m_filePath;
};

class FileReader : public IReader {
public:
    Awaitable run();
};

}
