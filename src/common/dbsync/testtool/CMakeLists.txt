cmake_minimum_required(VERSION 3.22)

project(dbsync_test_tool)

add_executable(dbsync_test_tool ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_include_directories(dbsync_test_tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include/
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/../src/)

target_link_libraries(dbsync_test_tool dbsync)

include(../../../cmake/ConfigureTarget.cmake)
configure_target(dbsync_test_tool)
