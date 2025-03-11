cmake_minimum_required(VERSION 3.22)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    project(network_helper)

    include(../../cmake/CommonSettings.cmake)
    set_common_settings()

    add_library(network_helper src/networkHelper.cpp src/networkUnixHelper.cpp)

    target_include_directories(network_helper PUBLIC include)

    target_link_libraries(network_helper PRIVATE string_helper)

    include(../../cmake/ConfigureTarget.cmake)
    configure_target(network_helper)
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
