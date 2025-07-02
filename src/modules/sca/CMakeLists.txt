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
    PUBLIC FilesystemWrapper # theres another wrapper in 4.x
           FileIO # readLineByLine and getFileContent are in 4.x
           dbsync # used extensively, available in 4.x, but no interface
           sysinfo # available in 4.x, uses processes method
           cmd_helper # Utils::exec differs from 6.x to 4.x, needs to be adapted
           nlohmann_json::nlohmann_json
    PRIVATE yaml-cpp::yaml-cpp
            PCRE2::8BIT
            hash_helper # uses Utils::HashData class available in 4.x hashHelper.h
            string_helper # uses Utils::Trim and Utils::asciiToHex (available in 4.x stringHelper.h)
            time_helper # uses Utils::getCurrentISO8601 available in 4.x timeHelper.h
            $<$<PLATFORM_ID:Windows>:registry_helper>) # uses Utils::Registry class (available) and Utils::KeyExists (not available)

include(../../cmake/ConfigureTarget.cmake)
configure_target(SCA)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
