cmake_minimum_required(VERSION 3.22)

project(Modules)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(inventory)

add_executable(modules src/main.cpp src/moduleManager.cpp)

target_include_directories(modules PRIVATE modules)

target_link_libraries(modules inventory)