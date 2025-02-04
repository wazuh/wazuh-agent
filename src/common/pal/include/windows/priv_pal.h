#pragma once

#ifndef PAL_H_REQUIRED
#error "Do not inclue priv_pal.h directly. Use pal.h instead"
#endif

#define PRIV_PAL_H_REQUIRED
#include <stdint.h>

#include "priv_pal_file_op.h"
#include "priv_pal_process.h"
#include "priv_pal_string.h"
#include "priv_pal_time.h"

#define ATTR_NONNULL
#define ATTR_NONNULL_ONE
#define ATTR_NONNULL_TWO
#define ATTR_NONNULL_ONE_TWO
#define ATTR_RET_NONNULL
#define ATTR_UNUSED
#define UNREFERENCED_PARAMETER(P) (P)

#pragma once

typedef int uid_t;
typedef int gid_t;
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
typedef uint8_t u_int8_t;
typedef int pid_t;

typedef int mode_t;
#define PATH_MAX 1024 * 1024
