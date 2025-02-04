#include <errno.h>
#include <shared.h>
// #include <error_messages.h>
#include <file_op.h>
#include <regex.h>

int check_path_type(const char* dir)
{
    DIR* dp;
    int retval;

    if (dp = opendir(dir), dp)
    {
        retval = 2;
        closedir(dp);
    }
    else if (errno == ENOTDIR)
    {
        retval = 1;
    }
    else
    {
        retval = 0;
    }
    return retval;
}

int cldir_ex_ignore(const char* name, const char** ignore)
{
    DIR* dir;
    struct dirent* dirent = NULL;
    char path[PATH_MAX + 1];

    // Erase content

    dir = opendir(name);

    if (!dir)
    {
        return -1;
    }

    while (dirent = readdir(dir), dirent)
    {
        // Skip "." and ".."
        // TODO: replace function w_str_in_array
        // if ((dirent->d_name[0] == '.' && (dirent->d_name[1] == '\0' || (dirent->d_name[1] == '.' && dirent->d_name[2]
        // == '\0'))) || w_str_in_array(dirent->d_name, ignore)) {
        if (dirent->d_name[0] == '.' &&
            (dirent->d_name[1] == '\0' || (dirent->d_name[1] == '.' && dirent->d_name[2] == '\0')))
        {
            continue;
        }

        if (snprintf(path, PATH_MAX + 1, "%s/%s", name, dirent->d_name) > PATH_MAX)
        {
            closedir(dir);
            return -1;
        }

        if (rmdir_ex(path) < 0)
        {
            closedir(dir);
            return -1;
        }
    }

    return closedir(dir);
}

char** wreaddir(const char* name)
{
    DIR* dir;
    struct dirent* dirent = NULL;
    char** files;
    unsigned int i = 0;

    if (dir = opendir(name), !dir)
    {
        return NULL;
    }

    os_malloc(sizeof(char*), files);

    while (dirent = readdir(dir), dirent)
    {
        // Skip "." and ".."
        if (dirent->d_name[0] == '.' &&
            (dirent->d_name[1] == '\0' || (dirent->d_name[1] == '.' && dirent->d_name[2] == '\0')))
        {
            continue;
        }

        os_realloc(files, (i + 2) * sizeof(char*), files);
        if (!files)
        {
            LogCritical(MEM_ERROR, errno, strerror(errno));
        }
        files[i++] = strdup(dirent->d_name);
    }

    files[i] = NULL;
    qsort(files, i, sizeof(char*), qsort_strcmp);
    closedir(dir);
    return files;
}
