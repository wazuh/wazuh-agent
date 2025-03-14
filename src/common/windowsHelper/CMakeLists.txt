cmake_minimum_required(VERSION 3.22)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    project(windows_helper)

    include(../../cmake/CommonSettings.cmake)
    set_common_settings()

    add_library(windows_helper src/windowsHelper.cpp)

    target_include_directories(windows_helper PUBLIC include)

    target_link_libraries(windows_helper PRIVATE string_helper time_helper pal)

    include(../../cmake/ConfigureTarget.cmake)
    configure_target(windows_helper)

    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
