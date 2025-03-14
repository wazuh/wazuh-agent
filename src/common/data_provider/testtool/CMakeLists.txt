cmake_minimum_required(VERSION 3.22)

project(sysinfo_test_tool)

include_directories(${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/src/)

add_executable(sysinfo_test_tool
               ${sysinfo_TESTTOOL_SRC}
               ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(sysinfo_test_tool
        sysinfo
        psapi
        iphlpapi
        ws2_32
    )
else()
    target_link_libraries(sysinfo_test_tool
        sysinfo
        dl
        pthread
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

include(../../../cmake/ConfigureTarget.cmake)
configure_target(sysinfo_test_tool)
