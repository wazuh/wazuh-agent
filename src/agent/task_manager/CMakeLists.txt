cmake_minimum_required(VERSION 3.22)

project(TaskManager)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)

add_library(TaskManager src/task_manager.cpp)

include(../../cmake/ConfigureTarget.cmake)
configure_target(TaskManager)

target_include_directories(TaskManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(TaskManager PUBLIC Boost::asio PRIVATE Logger)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
