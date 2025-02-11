cmake_minimum_required(VERSION 3.22)

project(pipeline_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_definitions(-DPROMISE_TYPE=PromiseType::NORMAL)

add_library(pipeline_helper INTERFACE)

target_include_directories(pipeline_helper INTERFACE include)

target_link_libraries(pipeline_helper INTERFACE utils thread_dispatcher)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
