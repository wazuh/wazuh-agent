cmake_minimum_required(VERSION 3.22)

project(SCA)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(SCA src/sca.cpp src/sca_policy_loader.cpp src/sca_event_handler.cpp)
target_include_directories(SCA PUBLIC include PRIVATE src)
target_link_libraries(
    SCA
    PUBLIC FilesystemWrapper
           dbsync
           Config
           ConfigurationParser
           MessageEntry
           ModuleManager
           CommandEntry)

include(../../cmake/ConfigureTarget.cmake)
configure_target(SCA)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
