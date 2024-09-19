/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * October 8, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#pragma once

#include <string>
#include <iostream>
#include <cstdio>
#include <memory>
#include <vector>

#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#else
FILE *popen(const char *command, const char *mode) { return NULL; }
int pclose(FILE *stream){ return 0; }
#endif

namespace Utils
{
    struct FileSmartDeleter
    {
        void operator()(FILE* file)
        {
            pclose(file);
        }
    };
    static std::string exec(const std::string& cmd, const size_t bufferSize = 128)
    {
        const std::unique_ptr<FILE, FileSmartDeleter> file{popen(cmd.c_str(), "r")};
        std::vector<char> buffer(bufferSize);
        std::string result;

        if (file)
        {
            while (fgets(buffer.data(), bufferSize, file.get()))
            {
                result += buffer.data();
            }
        }

        return result;
    }
}

#pragma GCC diagnostic pop

