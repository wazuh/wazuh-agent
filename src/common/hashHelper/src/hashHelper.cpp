#include "hashHelper.hpp"

#include <array>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Utils
{
    std::vector<unsigned char> hashFile(const std::string& filepath)
    {
        std::ifstream inputFile(filepath, std::fstream::in);
        if (inputFile.good())
        {
            constexpr int BUFFER_SIZE {4096};
            std::array<char, BUFFER_SIZE> buffer {};

            HashData hash;
            while (inputFile.read(buffer.data(), buffer.size()))
            {
                hash.update(buffer.data(), static_cast<size_t>(inputFile.gcount()));
            }
            hash.update(buffer.data(), static_cast<size_t>(inputFile.gcount()));

            return hash.hash();
        }

        throw std::runtime_error {"Unable to open '" + filepath + "' for hashing."};
    };
} // namespace Utils
