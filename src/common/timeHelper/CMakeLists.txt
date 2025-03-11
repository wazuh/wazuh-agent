cmake_minimum_required(VERSION 3.22)

project(time_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(time_helper src/timeHelper.cpp)

target_include_directories(time_helper PUBLIC include)

target_link_libraries(time_helper PRIVATE string_helper pal)

include(../../cmake/ConfigureTarget.cmake)
configure_target(time_helper)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
