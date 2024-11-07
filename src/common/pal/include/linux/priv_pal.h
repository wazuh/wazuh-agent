#pragma once

#ifndef PAL_H_REQUIRED
#error "Do not inclue priv_pal.h directly. Use pal.h instead"
#endif

#define PRIV_PAL_H_REQUIRED

#include "priv_pal_file.h"
#include "priv_pal_thread.h"
#include "priv_pal_time.h"

#define ATTR_NONNULL         __attribute__((nonnull))
#define ATTR_NONNULL_ONE     __attribute__((nonnull(1)))
#define ATTR_NONNULL_TWO     __attribute__((nonnull(2)))
#define ATTR_NONNULL_ONE_TWO __attribute__((nonnull(1, 2)))
#define ATTR_RET_NONNULL     __attribute__((__returns_nonnull__))
#define ATTR_UNUSED          __attribute__((unused))
#define UNREFERENCED_PARAMETER(P)
