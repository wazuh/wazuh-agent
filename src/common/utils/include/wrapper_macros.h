#ifndef WRAPPER_MACROS_H
#define WRAPPER_MACROS_H

#include "os_macros.h"

#define w_fclose(x) if (x) { fclose(x); x=NULL; }

#define w_strdup(x,y) ({ int retstr = 0; if (x) { os_strdup(x, y);} else retstr = 1; retstr;})

#define sqlite_strdup(x,y) ({ if (x) { os_strdup(x, y); } else (void)0; })

#define w_strlen(x) ((x)? strlen(x) : 0)

#endif // WRAPPER_MACROS_H
