cmake_minimum_required(VERSION 3.22)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    project(encoding_helper)

    include(../../cmake/CommonSettings.cmake)
    set_common_settings()

    add_library(encoding_helper src/encodingWindowsHelper.cpp)

    target_include_directories(encoding_helper PUBLIC include)

    include(../../cmake/ConfigureTarget.cmake)
    configure_target(encoding_helper)

    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
