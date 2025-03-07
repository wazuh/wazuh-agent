#include <errno.h>
#include <time.h>

struct tm* localtime_r(const time_t* timep, struct tm* result)
{
    if (timep == NULL || result == NULL)
    {
        return NULL;
    }

    errno_t err = localtime_s(result, timep);
    if (err != 0)
    {
        return NULL;
    }
    return result;
}

struct tm* gmtime_r(const time_t* timep, struct tm* result)
{
    if (timep == NULL || result == NULL)
    {
        return NULL;
    }

    errno_t err = gmtime_s(result, timep);
    if (err != 0)
    {
        return NULL;
    }
    return result;
}
