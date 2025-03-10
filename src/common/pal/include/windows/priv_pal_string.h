#pragma once

#if !defined(PRIV_PAL_H_REQUIRED) || !defined(PAL_H_REQUIRED)
#error "Do not include this file. Use pal.h instead"
#endif

#define strtok_r strtok_s
char** OS_StrBreak(char match, const char* _str, size_t size);
