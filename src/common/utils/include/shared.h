/* Copyright (C) 2015, Wazuh Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

/*
 *  The stack smashing protector defeats some BoF via: gcc -fstack-protector
 *  Reference: http://gcc.gnu.org/onlinedocs/gcc-4.1.2/cpp.pdf
 */

#pragma once

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1) && (__GNUC_PATCHLEVEL__ >= 2)) || \
                          ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2)) || \
                           (__GNUC__ >= 5))

/* Heuristically enable the stack protector on sensitive functions */
#define __SSP__ 1

/* FORTIFY_SOURCE is RedHat / Fedora specific */
#define FORTIFY_SOURCE
#endif

#ifndef LARGEFILE64_SOURCE
#define LARGEFILE64_SOURCE
#endif /* LARGEFILE64_SOURCE */

#ifndef FILE_OFFSET_BITS
#define FILE_OFFSET_BITS 64
#endif /* FILE_OFFSET_BITS */

/* Global headers */
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <sys/time.h>
#include <sys/param.h>
#endif
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#ifndef WIN32
#include <sys/wait.h>
#include <sys/resource.h>

// Only Linux and FreeBSD need mount.h */
#if defined(Linux)
#include <sys/mount.h>
#endif

#include <sys/utsname.h>
#endif /* NOT WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#include <dirent.h>
#endif
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>

#ifndef WIN32
#include <pthread.h>
#include <glob.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#else
#include <vcruntime.h>
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <ws2tcpip.h>
#include <shlwapi.h>
#include <direct.h>
#endif

#ifdef __cplusplus
#include <atomic>
#define _Atomic(T) std::atomic<T>
#endif

#include <time.h>
#include <errno.h>
#ifndef WIN32
#include <libgen.h>
#endif

#include "defs.h"
#include "os_err.h"

#ifndef LARGEFILE64_SOURCE
#define LARGEFILE64_SOURCE
#endif /* LARGEFILE64_SOURCE */

#ifndef FILE_OFFSET_BITS
#define FILE_OFFSET_BITS 64
#endif /* FILE_OFFSET_BITS */

/* Global portability code */

#ifdef Darwin
typedef int sock2len_t;
#endif

#ifndef WIN32
#define CloseSocket(x) close(x)
#endif

#ifdef WIN32
typedef int socklen_t;
#define sleep(x) Sleep((x) * 1000)
#define srandom(x) srand(x)
#define lstat(x,y) stat(x,y)
#define CloseSocket(x) closesocket(x)
void WinSetError();

#define MSG_DONTWAIT    0

#ifndef PROCESSOR_ARCHITECTURE_AMD64
#define PROCESSOR_ARCHITECTURE_AMD64 9
#endif
#endif /* WIN32 */

#if defined(__GNUC__) && __GNUC__ >= 7
#define WFALLTHROUGH __attribute__ ((fallthrough))
#else
#define WFALLTHROUGH ((void) 0)
#endif

/* Common structure for socket forwarding in Analysisd and logcollector */
typedef struct _socket_forwarder {
    char   *name;
    char   *location;
    int    mode;
    char   *prefix;
    int    socket;
    time_t last_attempt;
} socket_forwarder;


extern const char *__local_name;
/*** Global prototypes ***/
/*** These functions will exit on error. No need to check return code ***/

#include "os_macros.h"

#include "wrapper_macros.h"

// Calculate the number of elements within an array.
// Only static arrays allowed.
#define array_size(array) (sizeof(array)/sizeof(array[0]))

#ifndef WAZUH_UNIT_TESTING
#define FOREVER() 1
#endif

