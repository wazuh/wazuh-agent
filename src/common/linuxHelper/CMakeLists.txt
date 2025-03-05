cmake_minimum_required(VERSION 3.22)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    project(linux_helper)

    include(../../cmake/CommonSettings.cmake)
    set_common_settings()

    add_library(linux_helper INTERFACE)

    target_include_directories(linux_helper INTERFACE include)

    target_link_libraries(linux_helper INTERFACE cmd_helper FileIO string_helper)

    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
