cmake_minimum_required(VERSION 3.22)

project(SCA)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(SCA INTERFACE)
target_include_directories(SCA INTERFACE include)
target_link_libraries(
    SCA
    INTERFACE FilesystemWrapper
              dbsync
              Config
              ConfigurationParser
              MessageEntry
              ModuleManager
              CommandEntry)

# Uncomment when SCA stops being a header-only library include(../../cmake/ConfigureTarget.cmake) configure_target(SCA)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
