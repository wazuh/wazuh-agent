cmake_minimum_required(VERSION 3.22)

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
target_link_libraries(ConfigurationParser PRIVATE toml11::toml11)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
