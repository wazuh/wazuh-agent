cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

find_package(toml11 CONFIG REQUIRED)

project(ConfigurationParser)
message(STATUS "******* Project ConfigurationParser *********")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(include)
include_directories(${toml11_INCLUDE_DIRS})
message(STATUS "Include directories: ${toml11_INCLUDE_DIRS}")


if(BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)

    enable_testing()

    add_executable(ConfigurationParser_test
        tests/configuration_parser_test.cpp
        src/configuration_parser.cpp
    )

    target_include_directories(ConfigurationParser_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(ConfigurationParser_test PUBLIC toml11::toml11 GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
endif()

add_library(configurationParser src/configuration_parser.cpp)

set_target_properties(configurationParser PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories(configurationParser PUBLIC)
target_link_libraries(configurationParser PUBLIC toml11::toml11)
