cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Logcollector)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

get_filename_component(SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
get_filename_component(COMMON_FOLDER ${SRC_FOLDER}/common/ ABSOLUTE)

if(APPLE)
    find_package(fmt REQUIRED)
    add_subdirectory(src/macos_reader/oslogstore)
endif()
find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio system)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${COMMON_FOLDER}
    ${COMMON_FOLDER}/stringHelper/include
    ${COMMON_FOLDER}/timeHelper/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file_reader/include
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/src/journald_reader/include>
    $<$<PLATFORM_ID:Linux>:${SYSTEMD_INCLUDE_DIRS}>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/src/winevt_reader/include>
    $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/src/macos_reader/include>)

target_link_libraries(Logcollector
    PUBLIC
    ConfigurationParser
    MultiTypeQueue
    ModuleCommand
    Config
    Boost::asio
    Boost::system
    nlohmann_json::nlohmann_json
    PRIVATE
    $<$<PLATFORM_ID:Darwin>:OSLogStoreWrapper>
    $<$<PLATFORM_ID:Darwin>:fmt::fmt>
    Logger
    pal
    $<$<PLATFORM_ID:Linux>:systemd>
)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Logcollector)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
