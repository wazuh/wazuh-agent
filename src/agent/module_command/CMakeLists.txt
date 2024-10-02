cmake_minimum_required(VERSION 3.22)

project(ModuleCommand)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(ModuleCommand INTERFACE)

target_include_directories(ModuleCommand INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
