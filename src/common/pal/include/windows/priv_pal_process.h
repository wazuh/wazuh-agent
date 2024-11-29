#pragma once

#if !defined(PRIV_PAL_H_REQUIRED) || !defined(PAL_H_REQUIRED)
#error "Do not include this file. Use pal.h instead"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

FILE* popen(const char* command, const char* mode);
int pclose(FILE* stream);

#ifdef __cplusplus
}
#endif
