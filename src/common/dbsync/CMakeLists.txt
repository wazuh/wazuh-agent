cmake_minimum_required(VERSION 3.22)

project(dbsync)

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

if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif(APPLE)

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${COMMON_FOLDER})

add_definitions(-DPROMISE_TYPE=PromiseType::NORMAL)

link_directories(${SRC_FOLDER})

file(GLOB DBSYNC_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite/*.cpp")

add_library(dbsync STATIC
    ${DBSYNC_SRC}
    ${SRC_FOLDER}/${RESOURCE_OBJ})

target_include_directories(dbsync PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

target_link_libraries(dbsync
    PUBLIC
    deleter_helper
    map_wrapper
    pipeline_helper
    sqlite_wrapper
    string_helper
    thread_dispatcher
)

target_link_libraries(dbsync
    PUBLIC
    nlohmann_json::nlohmann_json
    cjson
    PRIVATE
    pal
    unofficial::sqlite3::sqlite3
)

add_subdirectory(example)
add_subdirectory(testtool)

include(../../cmake/ConfigureTarget.cmake)
configure_target(dbsync)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(integrationTests)
endif(BUILD_TESTS)
