cmake_minimum_required(VERSION 3.22)

project(SCA)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(
    SCA
    src/sca.cpp
    src/sca_policy.cpp
    src/sca_policy_check.cpp
    src/sca_policy_loader.cpp
    src/sca_event_handler.cpp
    src/sca_utils.cpp
    src/check_condition_evaluator.cpp)
target_include_directories(SCA PUBLIC include PRIVATE src)
target_link_libraries(
    SCA
    PUBLIC FilesystemWrapper
           FileIO
           dbsync
           Config
           ConfigurationParser
           MessageEntry
           ModuleManager
           CommandEntry
           cmd_helper
    PRIVATE OsUtils hash_helper string_helper time_helper)

include(../../cmake/ConfigureTarget.cmake)
configure_target(SCA)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
