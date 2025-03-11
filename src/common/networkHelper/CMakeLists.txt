cmake_minimum_required(VERSION 3.22)

project(network_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_library(network_helper src/networkWindowsHelper.cpp)

    target_include_directories(network_helper PUBLIC include)

    target_link_libraries(network_helper PRIVATE encoding_helper string_helper windows_helper iphlpapi ws2_32)
else()
    add_library(network_helper src/networkHelper.cpp src/networkUnixHelper.cpp)

    target_include_directories(network_helper PUBLIC include)

    target_link_libraries(network_helper PRIVATE string_helper)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

include(../../cmake/ConfigureTarget.cmake)
configure_target(network_helper)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
