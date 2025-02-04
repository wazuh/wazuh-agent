#include <file_op.h>
#include <shared.h>

int check_path_type(const char* dir)
{
    DWORD attributes = GetFileAttributes(dir);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return 0;
    }

    return (attributes & FILE_ATTRIBUTE_DIRECTORY) ? 2 : 1;
}

int cldir_ex_ignore(const char* name, const char** ignore)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char path[MAX_PATH + 1];
    char searchPattern[MAX_PATH + 1];

    // Create the search pattern
    snprintf(searchPattern, MAX_PATH + 1, "%s\\*", name);

    // Find the first file in the directory
    hFind = FindFirstFile(searchPattern, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    do
    {
        // Skip "." and ".."
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0)
        {
            continue;
        }

        // Create the full path
        if (snprintf(path, MAX_PATH + 1, "%s\\%s", name, findFileData.cFileName) > MAX_PATH)
        {
            FindClose(hFind);
            return -1;
        }

        // Remove the file or directory
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // Recursively remove the directory
            cldir_ex_ignore(path, ignore);
            RemoveDirectory(path);
        }
        else
        {
            // Remove the file
            DeleteFile(path);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    return FindClose(hFind) ? 0 : -1;
}

char** wreaddir(const char* name)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char** files = NULL;
    unsigned int i = 0;
    char searchPattern[MAX_PATH + 1];

    // Create the search pattern
    snprintf(searchPattern, MAX_PATH + 1, "%s\\*", name);

    // Find the first file in the directory
    hFind = FindFirstFile(searchPattern, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    os_malloc(sizeof(char*), files);

    do
    {
        // Skip "." and ".."
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0)
        {
            continue;
        }

        // Allocate more memory for the files array
        char** tempFiles = (char**)realloc(files, (i + 2) * sizeof(char*));
        if (!tempFiles)
        {
            LogCritical(MEM_ERROR, errno, strerror(errno));
            free(files);
            FindClose(hFind);
            return NULL;
        }

        files = tempFiles;
        files[i] = _strdup(findFileData.cFileName);
        if (!files[i])
        {
            free(files);
            FindClose(hFind);
            return NULL;
        }
        i++;
    } while (FindNextFile(hFind, &findFileData) != 0);

    files[i] = NULL;
    qsort(files, i, sizeof(char*), qsort_strcmp);

    FindClose(hFind);
    return files;
}
