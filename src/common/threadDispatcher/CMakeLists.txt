cmake_minimum_required(VERSION 3.22)

project(thread_dispatcher)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_definitions(-DPROMISE_TYPE=PromiseType::NORMAL)

add_library(thread_dispatcher INTERFACE)

target_include_directories(thread_dispatcher INTERFACE include ../)

target_link_libraries(thread_dispatcher INTERFACE promise_helper)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
