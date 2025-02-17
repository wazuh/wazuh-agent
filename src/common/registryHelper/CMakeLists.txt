cmake_minimum_required(VERSION 3.22)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    project(registry_helper)

    include(../../cmake/CommonSettings.cmake)
    set_common_settings()

    add_definitions(-DPROMISE_TYPE=PromiseType::NORMAL)

    add_library(registry_helper INTERFACE)

    target_include_directories(registry_helper INTERFACE include)

    target_link_libraries(registry_helper INTERFACE encoding_helper glob_helper string_helper time_helper windows_helper)

    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
