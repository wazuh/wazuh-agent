#include "utilsWrapper.hpp"
#include "cmdHelper.hpp"

std::string UtilsWrapper::exec(const std::string& cmd, const size_t bufferSize)
{
    return Utils::PipeOpen(cmd, bufferSize);
}
