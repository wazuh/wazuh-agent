cmake_minimum_required(VERSION 3.22)

project(ModuleCommand)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(nlohmann_json REQUIRED)

add_library(ModuleCommand INTERFACE)

target_include_directories(ModuleCommand INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ModuleCommand INTERFACE nlohmann_json::nlohmann_json)
