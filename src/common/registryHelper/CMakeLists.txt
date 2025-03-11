cmake_minimum_required(VERSION 3.22)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    project(registry_helper)

    include(../../cmake/CommonSettings.cmake)
    set_common_settings()

    add_library(registry_helper src/registryHelper.cpp)

    target_include_directories(registry_helper PUBLIC include)

    target_link_libraries(registry_helper PRIVATE encoding_helper glob_helper windows_helper)

    include(../../cmake/ConfigureTarget.cmake)
    configure_target(registry_helper)

    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
