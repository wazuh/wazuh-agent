cmake_minimum_required(VERSION 3.22)

project(SCA)

# compile with C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# include(../../cmake/CommonSettings.cmake)
# set_common_settings()

find_package(pcre2 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_library(
    SCA
    src/sca.cpp
    src/sca_policy.cpp
    src/sca_policy_check.cpp
    src/sca_policy_loader.cpp
    src/sca_policy_parser.cpp
    src/sca_event_handler.cpp
    src/sca_utils.cpp
    src/check_condition_evaluator.cpp
    $<$<PLATFORM_ID:Windows>:src/sca_policy_check_win.cpp>)

target_include_directories(SCA PUBLIC include PRIVATE src)

target_compile_definitions(SCA PRIVATE PCRE2_CODE_UNIT_WIDTH=8)

target_link_libraries(
    SCA
    PUBLIC FilesystemWrapper
           FileIO
           dbsync
           sysinfo
           MessageEntry
           CommandEntry
           cmd_helper
           nlohmann_json::nlohmann_json
    PRIVATE yaml-cpp::yaml-cpp PCRE2::8BIT hash_helper string_helper time_helper $<$<PLATFORM_ID:Windows>:registry_helper>)

include(../../cmake/ConfigureTarget.cmake)
configure_target(SCA)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
