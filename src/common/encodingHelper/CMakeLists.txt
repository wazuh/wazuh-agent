cmake_minimum_required(VERSION 3.22)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    project(encoding_helper)

    include(../../cmake/CommonSettings.cmake)
    set_common_settings()

    add_library(encoding_helper INTERFACE)

    target_include_directories(encoding_helper INTERFACE include)

    target_link_libraries(encoding_helper INTERFACE)

    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
