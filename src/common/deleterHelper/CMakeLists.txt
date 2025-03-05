cmake_minimum_required(VERSION 3.22)

project(deleter_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(deleter_helper INTERFACE)

target_include_directories(deleter_helper INTERFACE include)
