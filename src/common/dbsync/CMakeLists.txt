cmake_minimum_required(VERSION 3.22)

project(dbsync)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_definitions(-DPROMISE_TYPE=PromiseType::NORMAL)

file(GLOB DBSYNC_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite/*.cpp")

add_library(dbsync STATIC
    ${DBSYNC_SRC})

target_include_directories(dbsync
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

target_link_libraries(dbsync
    PUBLIC
    deleter_helper
    map_wrapper
    pipeline_helper
    sqlite_wrapper
    string_helper
    thread_dispatcher
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
