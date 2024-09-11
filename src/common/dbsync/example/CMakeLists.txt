cmake_minimum_required(VERSION 3.12.4)

project(dbsync_example)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
link_directories(${CMAKE_BINARY_DIR}/lib)

if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++14")

add_executable(dbsync_example
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" )

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(dbsync_example
        dbsync
        pthread
        -static-libgcc -static-libstdc++
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    target_link_libraries(dbsync_example
        dbsync
        pthread)
else()
    target_link_libraries(dbsync_example
        dbsync
        pthread
        dl
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
