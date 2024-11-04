cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(ConfigurationParser)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

include_directories(${CMAKE_SOURCE_DIR}/common/logger/include)

find_package(yaml-cpp CONFIG REQUIRED)

add_library(ConfigurationParser src/configuration_parser.cpp)
target_include_directories(ConfigurationParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ConfigurationParser PUBLIC yaml-cpp::yaml-cpp Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(ConfigurationParser)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
