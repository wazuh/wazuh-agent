#pragma once

#if !defined(PRIV_PAL_H_REQUIRED) || !defined(PAL_H_REQUIRED)
#error "Do not include this file. Use pal.h instead"
#endif
/*
#include <windows.h>
#include <stdint.h>
#include <stdio.h>

typedef struct DIR {
    HANDLE hFind;
    WIN32_FIND_DATA findFileData;
    int is_open;
}DIR;

typedef struct dirent {
    char d_name[MAX_PATH];
}dirent;

DIR *opendir(const char *dir);
int closedir(DIR *dir);
struct dirent *readdir(DIR *dir);
char *dirname(char *s);

int64_t ftello64(FILE *x);
int64_t fseeko64(FILE *x, int64_t pos, int mode);

int64_t S_ISREG(int64_t flags);
int64_t S_ISDIR(int64_t flags);

char *PathFindFileNameA_(char *s);
void PathRemoveFileSpec_(char *path);
*/
