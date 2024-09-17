#pragma once

#include <ilogger.hpp>

#include <spdlog/spdlog.h>

inline const char* GetFileName(const char* path)
{
    const char* file = strrchr(path, '/');
    return file ? file + 1 : path;
}

#define LOG_FILE_NAME GetFileName(__FILE__)

#define LogTrace(message, ...)                                                                                         \
    spdlog::trace("[TRACE] [{}:{}] [{}] " message, LOG_FILE_NAME, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#define LogDebug(message, ...)                                                                                         \
    spdlog::debug("[DEBUG] [{}:{}] [{}] " message, LOG_FILE_NAME, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#define LogInfo(message, ...)                                                                                          \
    spdlog::info("[INFO] [{}:{}] [{}] " message, LOG_FILE_NAME, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#define LogWarn(message, ...)                                                                                          \
    spdlog::warn("[WARN] [{}:{}] [{}] " message, LOG_FILE_NAME, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#define LogError(message, ...)                                                                                         \
    spdlog::error("[ERROR] [{}:{}] [{}] " message, LOG_FILE_NAME, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)
#define LogCritical(message, ...)                                                                                      \
    spdlog::critical("[CRITICAL] [{}:{}] [{}] " message, LOG_FILE_NAME, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)

class Logger : public Ilogger
{
public:
    Logger();
};
