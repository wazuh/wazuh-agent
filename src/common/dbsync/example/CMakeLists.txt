cmake_minimum_required(VERSION 3.22)

project(dbsync_example)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
link_directories(${CMAKE_BINARY_DIR}/lib)

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

include(../../../cmake/ConfigureTarget.cmake)
configure_target(dbsync_example)
