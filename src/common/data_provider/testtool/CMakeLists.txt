cmake_minimum_required(VERSION 3.22)

project(sysinfo_test_tool)

add_executable(sysinfo_test_tool ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_include_directories(sysinfo_test_tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include/
                                                     ${CMAKE_CURRENT_SOURCE_DIR}/../src/)

if(WIN32)
    target_link_libraries(sysinfo_test_tool sysinfo psapi iphlpapi ws2_32)
else()
    target_link_libraries(sysinfo_test_tool sysinfo)
endif()

include(../../../cmake/ConfigureTarget.cmake)
configure_target(sysinfo_test_tool)
