#pragma once

#if !defined(PRIV_PAL_H_REQUIRED) || !defined(PAL_H_REQUIRED)
#error "Do not include this file. Use pal.h instead"
#endif

int check_path_type(const char* dir);
int cldir_ex_ignore(const char* name, const char** ignore);
char** wreaddir(const char* name);
