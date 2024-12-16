#pragma once

#define PAL_H_REQUIRED

#if defined(__GNUC__)               // Using GNU C/C++ compiler.
    #include "linux/priv_pal.h"
#elif defined(__apple_build_version__)
    #include "macos/priv_pal.h"
#elif defined(_MSC_VER)
    #include "windows/priv_pal.h"
#endif
