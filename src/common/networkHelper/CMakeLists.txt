cmake_minimum_required(VERSION 3.22)

project(network_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(network_helper INTERFACE)

target_include_directories(network_helper INTERFACE include)
