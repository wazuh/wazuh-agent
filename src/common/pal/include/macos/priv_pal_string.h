#pragma once
#include <stddef.h>

#if !defined(PRIV_PAL_H_REQUIRED) || !defined(PAL_H_REQUIRED)
#error "Do not include this file. Use pal.h instead"
#endif

char** OS_StrBreak(char match, const char* _str, size_t size);
