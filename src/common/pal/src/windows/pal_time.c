#include <time.h>
#include <errno.h>

struct tm* localtime_r(const time_t* timep, struct tm* result) {
    // Llamar a localtime_s que es la versión segura de localtime en MSVC
    errno_t err = localtime_s(result, timep);
    if (err != 0) {
        return NULL; // Error en la conversión
    }
    return result; // Devuelve la estructura tm con el resultado
}
