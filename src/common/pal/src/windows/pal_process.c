#include <windows.h>
#include <memory>
#include <string>
#include <vector>
#include <cstdio>

FILE* popen(const char* command, const char* mode) {
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        return nullptr;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = (strcmp(mode, "r") == 0) ? hWritePipe : GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = si.hStdOutput;

    ZeroMemory(&pi, sizeof(pi));

    // Crear el proceso
    if (!CreateProcess(NULL, const_cast<LPSTR>(command), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);
        return nullptr;
    }

    // Cerramos el lado de escritura del pipe
    CloseHandle(hWritePipe);

    // Devolver el handle del pipe de lectura como FILE*
    return _fdopen(_open_osfhandle((intptr_t)hReadPipe, _O_RDONLY), "r");
}

int pclose(FILE* stream) {
    if (stream == nullptr) return -1;

    HANDLE hReadPipe = (HANDLE)_get_osfhandle(_fileno(stream));
    CloseHandle(hReadPipe);
    fclose(stream);

    // Aquí puedes agregar lógica para esperar el proceso y obtener su código de salida si es necesario
    return 0; // Deberías devolver el código de salida real del proceso
}
