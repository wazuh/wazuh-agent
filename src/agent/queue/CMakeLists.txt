cmake_minimum_required(VERSION 3.22)

project(queue_test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wunused -pthread")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(queue_test
	src/sqlitestorage.cpp
	src/queue.cpp
)

find_package(SQLiteCpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)

target_include_directories(queue_test PRIVATE include ${SQLiteCpp_INCLUDE_DIRS})
target_link_libraries(queue_test PRIVATE SQLiteCpp nlohmann_json::nlohmann_json fmt::fmt)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
