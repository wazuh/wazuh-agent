#include <logger.hpp>

#include <spdlog/sinks/win_eventlog_sink.h>

Logger::Logger()
{
    auto sink = std::make_shared<spdlog::sinks::win_eventlog_sink_st>("Wazuh-Agent");
    auto logger = std::make_shared<spdlog::logger>("wazuh-agent", sink);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
}

#ifdef __cplusplus
extern "C" {
#endif

void LogTrace_C(const char* file, int line, const char* func, const char* message, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, LOG_BUFFER_SIZE, message, args);
    va_end(args);
    spdlog::trace("[TRACE] [{}:{}] [{}] {}", file, line, func, buffer);
};

void LogDebug_C(const char* file, int line, const char* func, const char* message, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, LOG_BUFFER_SIZE, message, args);
    va_end(args);
    spdlog::debug("[DEBUG] [{}:{}] [{}] {}", file, line, func, buffer);
};

void LogInfo_C(const char* file, int line, const char* func, const char* message, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, LOG_BUFFER_SIZE, message, args);
    va_end(args);
    spdlog::info("[INFO] [{}:{}] [{}] {}", file, line, func, buffer);
};

void LogWarn_C(const char* file, int line, const char* func, const char* message, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, LOG_BUFFER_SIZE, message, args);
    va_end(args);
    spdlog::warn("[WARN] [{}:{}] [{}] {}", file, line, func, buffer);
};

void LogError_C(const char* file, int line, const char* func, const char* message, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, LOG_BUFFER_SIZE, message, args);
    va_end(args);
    spdlog::error("[ERROR] [{}:{}] [{}] {}", file, line, func, buffer);
};

void LogCritical_C(const char* file, int line, const char* func, const char* message, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, LOG_BUFFER_SIZE, message, args);
    va_end(args);
    spdlog::critical("[CRITICAL] [{}:{}] [{}] {}", file, line, func, buffer);
};

#ifdef __cplusplus
}
#endif
