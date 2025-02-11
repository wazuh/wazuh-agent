cmake_minimum_required(VERSION 3.22)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    project(windows_helper)

    include(../../cmake/CommonSettings.cmake)
    set_common_settings()

    add_definitions(-DPROMISE_TYPE=PromiseType::NORMAL)

    add_library(windows_helper INTERFACE)

    target_include_directories(windows_helper INTERFACE include)

    target_link_libraries(windows_helper INTERFACE encoding_helper string_helper time_helper pal)

    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
