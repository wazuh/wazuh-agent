cmake_minimum_required(VERSION 3.22)

project(sysinfo)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

get_filename_component(SRC_FOLDER     ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
get_filename_component(COMMON_FOLDER  ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(UNIX AND NOT APPLE)
    find_package(Lua REQUIRED)
    include_directories(${LUA_INCLUDE_DIR})
endif()

if(APPLE)
    find_package(unofficial-libplist CONFIG REQUIRED)
endif()

find_package(cJSON CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

find_path(POPT_INCLUDE_DIR include/popt.h)
include_directories(${POPT_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${COMMON_FOLDER})

link_directories(${SRC_FOLDER})

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utilsWrapper.cpp
    ${SRC_FOLDER}/${RESOURCE_OBJ})

target_include_directories(sysinfo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

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
)

if(WIN32)
    target_link_libraries(sysinfo PUBLIC encoding_helper registry_helper windows_helper)
elseif(NOT APPLE)
    target_link_libraries(sysinfo PUBLIC linux_helper)
endif()

if(WIN32)
    target_link_libraries(sysinfo
        PUBLIC
        psapi
        iphlpapi
        ws2_32
    )
elseif(APPLE)
    find_library(iokit_lib IOKit REQUIRED)
    find_library(corefoundation_lib CoreFoundation REQUIRED)

    target_link_libraries(sysinfo
        PUBLIC
        unofficial::libplist::libplist
        unofficial::libplist::libplist++
        ${iokit_lib}
        ${corefoundation_lib}
        LibArchive::LibArchive
    )
else()
    find_library(POPT_LIBRARY libpopt.a)
    find_library(PROC_NG_LIBRARY NAMES proc-ng REQUIRED)
    find_library(LIBDB_LIBRARY NAMES db REQUIRED)
    find_library(LIBRPM_LIBRARY NAMES rpm REQUIRED)
    find_library(LIBRPMIO_LIBRARY NAMES rpmio REQUIRED)

    target_link_libraries(sysinfo
        PUBLIC
        ${LIBDB_LIBRARY}
        ${PROC_NG_LIBRARY}
        ${LIBRPM_LIBRARY}
        ${LIBRPMIO_LIBRARY}
        LibArchive::LibArchive
        ${POPT_LIBRARY}
        ${LUA_LIBRARIES}
    )
endif()

target_link_libraries(sysinfo
    PUBLIC
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
