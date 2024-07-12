cmake_minimum_required(VERSION 3.12.4)

project(Communicator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Enable testing
enable_testing()

include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/communicator)
