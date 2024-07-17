cmake_minimum_required(VERSION 3.18)

# Set the project name and C++ standard
project(QueueTest LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find the nlohmann::json and SQLiteCpp package using vcpkg
find_package(SQLiteCpp REQUIRED)
find_package(nlohmann_json REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wunused -pthread")

# Add the include directory for shared headers
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(tests)

# Add the executable
add_executable(QueueTest
	src/main.cpp
	src/sqlitestorage.cpp
	)

# Link the nlohmann::json library to your project
target_include_directories(QueueTest PRIVATE include ${SQLiteCpp_INCLUDE_DIRS})
target_link_libraries(QueueTest PRIVATE SQLiteCpp nlohmann_json::nlohmann_json)
