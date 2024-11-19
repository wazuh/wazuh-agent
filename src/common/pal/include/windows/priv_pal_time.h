#pragma once

#include <time.h>

#if !defined(PRIV_PAL_H_REQUIRED) || !defined(PAL_H_REQUIRED)
#error "Do not include this file. Use pal.h instead"
#endif

struct tm *localtime_r(const time_t *timer, struct tm *result);
