#pragma once

#include <time.h>

#if !defined(PRIV_PAL_H_REQUIRED) || !defined(PAL_H_REQUIRED)
#error "Do not include this file. Use pal.h instead"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    struct tm* localtime_r(const time_t* timer, struct tm* result);
    struct tm* gmtime_r(const time_t* timep, struct tm* result);

#ifdef __cplusplus
}
#endif
