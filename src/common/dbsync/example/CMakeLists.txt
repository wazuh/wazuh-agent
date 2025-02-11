cmake_minimum_required(VERSION 3.22)

project(dbsync_example)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
link_directories(${CMAKE_BINARY_DIR}/lib)

if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++14")
else()
  set(CMAKE_CXX_FLAGS "/W4 /permissive- /std:c++14 /MT")
endif()

add_executable(dbsync_example
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" )

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(dbsync_example
        dbsync
    )
else()
    target_link_libraries(dbsync_example
        dbsync
        pthread
        dl
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
