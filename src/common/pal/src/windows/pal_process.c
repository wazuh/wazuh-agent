#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>

typedef struct {
    HANDLE hProcess;
    FILE* pipeStream;
} PopenHandle;

PopenHandle* popen(const char* command, const char* mode) {
    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

    // Create a pipe
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        return NULL;
    }

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;

    if (strcmp(mode, "r") == 0) {
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    } else if (strcmp(mode, "w") == 0) {
        si.hStdInput = hReadPipe;
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    } else {
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return NULL;
    }

    // Create the process
    if (!CreateProcess(NULL, (LPSTR)command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return NULL;
    }

    // Close the unused end of the pipe
    if (strcmp(mode, "r") == 0) {
        CloseHandle(hWritePipe);
    } else {
        CloseHandle(hReadPipe);
    }

    // Convert the HANDLE to a FILE*
    FILE* pipeStream = _fdopen(_open_osfhandle((intptr_t)(strcmp(mode, "r") == 0 ? hReadPipe : hWritePipe), _O_BINARY), mode);
    if (!pipeStream) {
        CloseHandle(pi.hProcess);
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return NULL;
    }

    // Store the process handle and stream in a custom struct
    PopenHandle* popenHandle = (PopenHandle*)malloc(sizeof(PopenHandle));
    popenHandle->hProcess = pi.hProcess;
    popenHandle->pipeStream = pipeStream;

    return popenHandle;
}

int pclose(PopenHandle* handle) {
    if (!handle || !handle->pipeStream) {
        return -1;
    }

    // Close the FILE* stream
    fclose(handle->pipeStream);

    // Wait for the process to exit
    WaitForSingleObject(handle->hProcess, INFINITE);

    // Get the exit code
    DWORD exitCode = 0;
    if (!GetExitCodeProcess(handle->hProcess, &exitCode)) {
        exitCode = -1; // Error occurred
    }

    // Close the process handle
    CloseHandle(handle->hProcess);
    free(handle);

    return (int)exitCode;
}
