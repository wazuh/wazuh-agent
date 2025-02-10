#pragma once

#if !defined(PRIV_PAL_H_REQUIRED) || !defined(PAL_H_REQUIRED)
#error "Do not include this file. Use pal.h instead"
#endif

/**
 * @brief Get the type of the specified file.
 *
 * @param dir File path.
 * @return 1 if it is a file, 2 if it is a directory, 0 otherwise.
 */
int check_path_type(const char* dir);

/**
 * @brief Delete directory content with exception list.
 *
 * @param name Path of the folder.
 * @param ignore Array of files to be ignored. This array must be NULL terminated.
 * @return 0 on success. On error, -1 is returned, and errno is set appropriately.
 */
int cldir_ex_ignore(const char* name, const char** ignore);

/**
 * @brief Read directory and return an array of contained files and folders, sorted alphabetically.
 *
 * @param name Path of the directory.
 * @return Array of filenames.
 */
char** wreaddir(const char* name);
