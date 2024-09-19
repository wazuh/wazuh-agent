/* Copyright (C) 2015, Wazuh Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All right reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "os_regex.h"
#include "os_regex_internal.h"

#if defined(__GNUC__) || defined(__clang__)
#define ATTR_NONNULL __attribute__((nonnull))
#define UNREFERENCED_PARAMETER(P)
#define ATTR_UNUSED __attribute__((unused))
#else
#define UNREFERENCED_PARAMETER(P) (P)
#define ATTR_UNUSED
#define ATTR_NONNULL
#endif

int _OS_Match(const char *pattern, const char *str, size_t str_len, size_t size)
{
    size_t i = 0, j;
    const char *pt = pattern;

    if (str_len < size) {
        return (FALSE);
    }

    size = str_len - size;

    /* Look to match the first pattern */
    do {
        /* Match */
        if (charmap[(uchar)str[i]] == *pt) {
            pt++;
            j = i + 1;

            while (*pt != '\0') {
                if (str[j] == '\0') {
                    return (FALSE);
                }

                else if (*pt != charmap[(uchar)str[j]]) {
                    pt = pattern;
                    goto nnext;
                }
                j++;
                pt++;
            }
            return (TRUE);
nnext:
            continue;
        }
    } while (++i <= size);

    return (FALSE);
}

int _os_strncmp(const char *pattern, const char *str, ATTR_UNUSED size_t str_len, size_t size)
{
    UNREFERENCED_PARAMETER(str_len);
    if (strncasecmp(pattern, str, size) == 0) {
        return (TRUE);
    }

    return (FALSE);
}

int _os_strcmp(const char *pattern, const char *str, ATTR_UNUSED size_t str_len, ATTR_UNUSED size_t size)
{
    UNREFERENCED_PARAMETER(str_len);
    if (strcasecmp(pattern, str) == 0) {
        return (TRUE);
    }

    return (FALSE);
}

int _os_strmatch(ATTR_UNUSED const char *pattern, ATTR_UNUSED const char *str,
                 ATTR_UNUSED size_t str_len, ATTR_UNUSED size_t size)
{
    UNREFERENCED_PARAMETER(pattern);
    UNREFERENCED_PARAMETER(str);
    UNREFERENCED_PARAMETER(str_len);
    UNREFERENCED_PARAMETER(size);
    return (TRUE);
}

int _os_strcmp_last(const char *pattern, const char *str, size_t str_len, size_t size)
{
    /* Size of the string must be bigger */
    if (str_len < size) {
        return (FALSE);
    }

    if (strcasecmp(pattern, str + (str_len - size)) == 0) {
        return (TRUE);
    }

    return (FALSE);
}

/* Compare an already compiled pattern with a not NULL string.
 * Returns 1 on success or 0 on error.
 * The error code is set on reg->error.
 */
int OSMatch_Execute(const char *str, size_t str_len, OSMatch *reg)
{
    short int i = 0;

    /* The reg can't be NULL */
    if(!reg){
        return (0);
    }

    /* The string can't be NULL */
    if (str == NULL) {
        reg->error = OS_REGEX_STR_NULL;
        return (0);
    }

    /* Loop over all sub patterns */
    while (reg->patterns[i]) {
        if (reg->match_fp[i](reg->patterns[i],
                             str,
                             str_len,
                             reg->size[i]) == TRUE) {
            return(reg->negate == 0?1:0);
        }
        i++;
    }

    return(reg->negate);
}
