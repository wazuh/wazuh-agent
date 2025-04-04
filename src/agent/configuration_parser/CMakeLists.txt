cmake_minimum_required(VERSION 3.22)

project(ConfigurationParser)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(yaml-cpp CONFIG REQUIRED)

add_library(ConfigurationParser src/configuration_parser.cpp src/configuration_parser_utils.cpp src/yaml_utils.cpp)
target_include_directories(ConfigurationParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ConfigurationParser PUBLIC yaml-cpp::yaml-cpp Logger PRIVATE Config)

include(../../cmake/ConfigureTarget.cmake)
configure_target(ConfigurationParser)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
