#pragma once

#include "customDeleter.hpp"
#include <cstdio>
#include <pal.h>

inline void pcloseWrapper(FILE* file)
{
    if (file)
    {
        pclose(file);
    }
}

struct FileSmartDeleter final : CustomDeleter<decltype(&pcloseWrapper), &pcloseWrapper>
{
};
