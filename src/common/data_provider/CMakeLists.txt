cmake_minimum_required(VERSION 3.22)

project(sysinfo)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

if(UNIX)
    find_package(LibArchive REQUIRED)
    if (APPLE)
        find_package(unofficial-libplist CONFIG REQUIRED)
        find_library(iokit_lib IOKit REQUIRED)
        find_library(corefoundation_lib CoreFoundation REQUIRED)
    else()
        find_package(Lua REQUIRED)
        find_path(POPT_INCLUDE_DIR include/popt.h)
        find_library(POPT_LIBRARY libpopt.a)
        find_library(PROC_NG_LIBRARY NAMES proc-ng REQUIRED)
        find_library(LIBDB_LIBRARY NAMES db REQUIRED)
        find_library(LIBRPM_LIBRARY NAMES rpm REQUIRED)
        find_library(LIBRPMIO_LIBRARY NAMES rpmio REQUIRED)
    endif()
endif()

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

if(WIN32)
    file(GLOB SYSINFO_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*Win.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/osinfo/sysOsInfoWin.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*Windows.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/*Windows.cpp")
elseif(APPLE)
    if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm64.*|ARM64.*")
        file(GLOB SYSINFO_SRC
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*Mac.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*CommonBSD.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/*Mac.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*BSD.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/osinfo/sysOsParsers.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/hardware/*ARMMac.cpp")
    else()
        file(GLOB SYSINFO_SRC
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*Mac.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*CommonBSD.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/*Mac.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*BSD.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/osinfo/sysOsParsers.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/hardware/*X86_64Mac.cpp")
    endif()
else()
    file(GLOB SYSINFO_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*Linux.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*Linux.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/osinfo/sysOsParsers.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/packageLinux*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/rpm*.cpp")
endif()

add_library(sysinfo STATIC
    ${SYSINFO_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sysInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utilsWrapper.cpp)

target_include_directories(sysinfo
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    $<$<PLATFORM_ID:Linux>:${LUA_INCLUDE_DIR}>
    $<$<PLATFORM_ID:Linux>:${POPT_INCLUDE_DIR}>
)

target_link_libraries(sysinfo
    PUBLIC
    byte_array_helper
    cmd_helper
    deleter_helper
    FilesystemWrapper
    FileIO
    json_helper
    network_helper
    sqlite_wrapper
    string_helper
    time_helper
    $<$<PLATFORM_ID:Windows>:encoding_helper>
    $<$<PLATFORM_ID:Windows>:registry_helper>
    $<$<PLATFORM_ID:Windows>:windows_helper>
    $<$<PLATFORM_ID:Windows>:psapi>
    $<$<PLATFORM_ID:Windows>:iphlpapi>
    $<$<PLATFORM_ID:Windows>:ws2_32>
    $<$<PLATFORM_ID:Darwin>:unofficial::libplist::libplist>
    $<$<PLATFORM_ID:Darwin>:unofficial::libplist::libplist++>
    $<$<PLATFORM_ID:Darwin>:${iokit_lib}>
    $<$<PLATFORM_ID:Darwin>:${corefoundation_lib}>
    $<$<PLATFORM_ID:Darwin>:LibArchive::LibArchive>
    $<$<PLATFORM_ID:Linux>:linux_helper>
    $<$<PLATFORM_ID:Linux>:${LIBDB_LIBRARY}>
    $<$<PLATFORM_ID:Linux>:${PROC_NG_LIBRARY}>
    $<$<PLATFORM_ID:Linux>:${LIBRPM_LIBRARY}>
    $<$<PLATFORM_ID:Linux>:${LIBRPMIO_LIBRARY}>
    $<$<PLATFORM_ID:Linux>:LibArchive::LibArchive>
    $<$<PLATFORM_ID:Linux>:${POPT_LIBRARY}>
    $<$<PLATFORM_ID:Linux>:${LUA_LIBRARIES}>
    nlohmann_json::nlohmann_json
    cjson
    PRIVATE
    pal
    unofficial::sqlite3::sqlite3
)

add_subdirectory(testtool)

include(../../cmake/ConfigureTarget.cmake)
configure_target(sysinfo)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(BUILD_TESTS)
