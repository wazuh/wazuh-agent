#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>

FILE* popen(const char* command, const char* mode) {
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

    // Store the process handle in a global/static variable or a custom structure
    SetFilePointer(GetStdHandle(STD_OUTPUT_HANDLE), 0, NULL, FILE_END);
    return pipeStream;
}

int pclose(FILE* stream) {
    if (!stream) {
        return -1;
    }

    // Retrieve the process handle associated with the FILE*
    intptr_t handleValue = _get_osfhandle(_fileno(stream));
    HANDLE hProcess = (HANDLE)handleValue;

    if (hProcess == INVALID_HANDLE_VALUE) {
        return -1;
    }

    // Close the FILE* stream
    fclose(stream);

    // Wait for the process to exit
    WaitForSingleObject(hProcess, INFINITE);

    // Get the exit code
    DWORD exitCode = 0;
    if (!GetExitCodeProcess(hProcess, &exitCode)) {
        exitCode = -1; // Error occurred
    }

    // Close the process handle
    CloseHandle(hProcess);

    return (int)exitCode;
}