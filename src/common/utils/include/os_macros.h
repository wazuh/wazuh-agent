#pragma once
// TODO: Double check this Critical error handling
/* for calloc: x = calloc(4,sizeof(char)) -> os_calloc(4,sizeof(char),x) */

#include <errno.h>
#include <stdlib.h>
#include "error_messages.h"

#define os_calloc(x,y,z) ((z = (__typeof__(z)) calloc(x,y)))?(void)1:LogCritical(MEM_ERROR, strerror(errno))

#define os_strdup(x,y) ((y = strdup(x)))?(void)1:LogCritical(MEM_ERROR, strerror(errno))

#define os_malloc(x,y) ((y = (__typeof__(y)) malloc(x)))?(void)1:LogCritical(MEM_ERROR, strerror(errno))

#define os_free(x) if(x){free(x);x=NULL;}

#define os_realloc(x,y,z) ((z = (__typeof__(z))realloc(x,y)))?(void)1:LogCritical(MEM_ERROR, strerror(errno))

#define os_clearnl(x,p) if((p = strrchr(x, '\n')))*p = '\0';
