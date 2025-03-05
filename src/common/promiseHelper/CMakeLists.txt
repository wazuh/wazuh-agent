cmake_minimum_required(VERSION 3.22)

project(promise_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(promise_helper INTERFACE)

target_include_directories(promise_helper INTERFACE include)
