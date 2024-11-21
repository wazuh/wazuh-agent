# Project name
set(PROJECT_NAME "Wazuh Agent")

# Project version
set(VERSION "0.1")

set(DEFAULT_THREAD_COUNT 4 CACHE STRING "Default number of threads (4)")

string(REPLACE "\\" "\\\\" DEFAULT_DATA_PATH "${DATA_INSTALL_DIR}")

string(REPLACE "\\" "\\\\" DEFAULT_RUN_PATH "${RUN_INSTALL_DIR}")

set(DEFAULT_SERVER_URL "https://localhost:27000" CACHE STRING "Default Agent Server Url")

set(DEFAULT_RETRY_INTERVAL 30000 CACHE STRING "Default Agent retry interval (30s)")

set(DEFAULT_BATCH_INTERVAL 10000 CACHE STRING "Default Agent batch interval (10s)")

set(DEFAULT_BATCH_SIZE 1000 CACHE STRING "Default Agent batch size limit (1000)")

set(DEFAULT_LOGCOLLECTOR_ENABLED true CACHE BOOL "Default Logcollector enabled")

set(BUFFER_SIZE 4096 CACHE STRING "Default Logcollector reading buffer size")

set(DEFAULT_FILE_WAIT 500 CACHE STRING "Default Logcollector file reading interval (500ms)")

set(DEFAULT_RELOAD_INTERVAL 60000 CACHE STRING "Default Logcollector reload interval (1m)")

set(DEFAULT_INVENTORY_ENABLED true CACHE BOOL "Default inventory enabled")

set(DEFAULT_INTERVAL 3600000 CACHE STRING "Default inventory interval (1h)")

set(DEFAULT_SCAN_ON_START true CACHE BOOL "Default inventory scan on start")

set(DEFAULT_HARDWARE true CACHE BOOL "Default inventory hardware")

set(DEFAULT_OS true CACHE BOOL "Default inventory os")

set(DEFAULT_NETWORK true CACHE BOOL "Default inventory network")

set(DEFAULT_PACKAGES true CACHE BOOL "Default inventory packages")

set(DEFAULT_PORTS true CACHE BOOL "Default inventory ports")

set(DEFAULT_PORTS_ALL true CACHE BOOL "Default inventory ports all")

set(DEFAULT_PROCESSES true CACHE BOOL "Default inventory processes")

set(DEFAULT_HOTFIXES true CACHE BOOL "Default inventory hotfixes")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SHARED_CONFIG_DIR "/etc/wazuh-agent/shared")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SHARED_CONFIG_DIR "/etc/wazuh-agent/shared")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(DEFINED ENV{ProgramData})
        set(SHARED_CONFIG_DIR "$ENV{ProgramData}\\wazuh-agent\\config\\shared")
    else()
        set(SHARED_CONFIG_DIR "C:\\ProgramData\\wazuh-agent\\config\\shared")
    endif()
else()
    message(FATAL_ERROR "Not supported OS")
endif()
