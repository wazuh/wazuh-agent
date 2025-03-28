cmake_minimum_required(VERSION 3.22)

project(Logcollector)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
    find_package(fmt REQUIRED)
    add_subdirectory(src/macos_reader/oslogstore)
endif()

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio)

FILE(GLOB LOGCOLLECTOR_SOURCES src/*.cpp src/file_reader/src/*.cpp)
FILE(GLOB JOURNALD_SOURCES src/journald_reader/src/*.cpp)
FILE(GLOB MACOS_SOURCES src/macos_reader/src/*.cpp)
FILE(GLOB WIN_SOURCES src/winevt_reader/src/*.cpp)

if(WIN32)
    FILE(GLOB_RECURSE EXCLUDED_SOURCES *_unix.cpp *_osx.cpp)
    list(APPEND LOGCOLLECTOR_SOURCES ${WIN_SOURCES})
elseif(APPLE)
    FILE(GLOB_RECURSE EXCLUDED_SOURCES *_win.cpp src/logcollector_unix.cpp)
    list(APPEND LOGCOLLECTOR_SOURCES ${MACOS_SOURCES})
else()
    FILE(GLOB_RECURSE EXCLUDED_SOURCES *_win.cpp *_osx.cpp)
    list(APPEND LOGCOLLECTOR_SOURCES ${JOURNALD_SOURCES})

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SYSTEMD REQUIRED libsystemd)
endif()

list(REMOVE_ITEM LOGCOLLECTOR_SOURCES ${EXCLUDED_SOURCES})

add_library(Logcollector ${LOGCOLLECTOR_SOURCES})

target_include_directories(Logcollector PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file_reader/include
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/src/journald_reader/include>
    $<$<PLATFORM_ID:Linux>:${SYSTEMD_INCLUDE_DIRS}>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/winevt_reader/include>
    $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/src/macos_reader/include>)

target_link_libraries(Logcollector
    PUBLIC
    ModuleManager
    ConfigurationParser
    MessageEntry
    CommandEntry
    string_helper
    time_helper
    Boost::asio
    nlohmann_json::nlohmann_json
    PRIVATE
    Config
    $<$<PLATFORM_ID:Darwin>:OSLogStoreWrapper>
    $<$<PLATFORM_ID:Darwin>:fmt::fmt>
    Logger
    $<$<PLATFORM_ID:Linux>:systemd>
)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Logcollector)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
