#pragma once

#ifdef WIN32

typedef int uid_t;
typedef int gid_t;
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
typedef uint8_t u_int8_t;
typedef int pid_t;

typedef int mode_t;
#define PATH_MAX 1024*1024
#endif
