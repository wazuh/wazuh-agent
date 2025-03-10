cmake_minimum_required(VERSION 3.22)

project(cmd_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(cmd_helper src/cmdHelper.cpp)

target_include_directories(cmd_helper PUBLIC include)

target_link_libraries(cmd_helper PRIVATE deleter_helper)

include(../../cmake/ConfigureTarget.cmake)
configure_target(cmd_helper)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
