#pragma once

#ifndef PAL_H_REQUIRED
#error "Do not inclue priv_pal.h directly. Use pal.h instead"
#endif

#define PRIV_PAL_H_REQUIRED

#include "priv_pal_file.h"
#include "priv_pal_thread.h"
#include "priv_pal_time.h"

#define ATTR_NONNULL
#define ATTR_NONNULL_ONE
#define ATTR_NONNULL_TWO
#define ATTR_NONNULL_ONE_TWO
#define ATTR_RET_NONNULL
#define ATTR_UNUSED
#define UNREFERENCED_PARAMETER(P) (P)
