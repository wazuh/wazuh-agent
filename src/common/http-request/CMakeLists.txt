cmake_minimum_required(VERSION 3.12.4)
project(urlrequest)

if(NOT SRC_FOLDER)
    get_filename_component(SRC_FOLDER     ${CMAKE_SOURCE_DIR}/ ABSOLUTE)
endif(NOT SRC_FOLDER)

message(STATUS "SRC_FOLDER ${SRC_FOLDER}")

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-fPIC")

set(BENCHMARK_ENABLE_TESTING "OFF")

find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include_directories(src/)
include_directories(include/)
include_directories(BEFORE PRIVATE shared/)

file(GLOB URL_REQUEST_SRC src/*.cpp)

add_library(urlrequest ${URL_REQUEST_SRC})

target_link_libraries(urlrequest nlohmann_json::nlohmann_json CURL::libcurl)

if (${CMAKE_PROJECT_NAME} STREQUAL "urlrequest")
    # Enable testing only if compiling this repository.
    # Always set enable_testing() before add_subdirectory.
    enable_testing()

    add_subdirectory(external/googletest)
    add_subdirectory(external/benchmark)

    add_subdirectory(test_tool)
    add_subdirectory(test)
endif (${CMAKE_PROJECT_NAME} STREQUAL "urlrequest")
