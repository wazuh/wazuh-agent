# Project name
set(PROJECT_NAME "Wazuh Agent")

# Project version
set(VERSION "0.1")

set(DEFAULT_SERVER_URL "https://localhost:27000" CACHE STRING "Default Agent Server Url")

set(DEFAULT_REGISTRATION_URL "https://localhost:55000" CACHE STRING "Default Agent Registration Url")

set(DEFAULT_RETRY_INTERVAL 30 CACHE STRING "Default Agent retry interval")

set(BUFFER_SIZE 4096 CACHE STRING "Default Logcollector reading buffer size")

set(DEFAULT_FILE_WAIT 500 CACHE STRING "Default Logcollector file reading interval (ms)")

set(DEFAULT_RELOAD_INTERVAL 60 CACHE STRING "Default Logcollector reload interval (sec)")

set(DEFAULT_LOGCOLLECTOR_ENABLED true CACHE BOOL "Default Logcollector enabled")

set(DEFAULT_LOCALFILES "/var/log/auth.log" CACHE PATH "Default localfiles")

set(DEFAULT_INVENTORY_ENABLED true CACHE BOOL "Default inventory enabled")

set(DEFAULT_INTERVAL 3600 CACHE STRING "Default inventory interval")

set(DEFAULT_SCAN_ON_START true CACHE STRING "Default inventory scan on start")

set(DEFAULT_HARDWARE true CACHE BOOL "Default inventory hardware")

set(DEFAULT_OS true CACHE BOOL "Default inventory os")

set(DEFAULT_NETWORK true CACHE BOOL "Default inventory network")

set(DEFAULT_PACKAGES true CACHE BOOL "Default inventory packages")

set(DEFAULT_PORTS true CACHE BOOL "Default inventory ports")

set(DEFAULT_PORTS_ALL true CACHE BOOL "Default inventory ports all")

set(DEFAULT_PROCESSES true CACHE BOOL "Default inventory processes")

set(DEFAULT_HOTFIXES true CACHE BOOL "Default inventory hotfixes")
