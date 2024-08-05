cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(ConfigurationParser)

find_package(toml11 CONFIG REQUIRED)

add_library(ConfigurationParser src/configuration_parser.cpp)

target_include_directories(ConfigurationParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ConfigurationParser PUBLIC toml11::toml11)

if(BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)

    enable_testing()

    add_executable(ConfigurationParser_test tests/configuration_parser_test.cpp)
    target_link_libraries(ConfigurationParser_test PUBLIC ConfigurationParser GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
    add_test(NAME ConfigParserTest COMMAND ConfigurationParser_test)
endif()
