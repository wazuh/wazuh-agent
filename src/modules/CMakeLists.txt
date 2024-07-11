cmake_minimum_required(VERSION 3.10)

project(Modules)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(modules/inventory)

add_executable(Modules
    modules/main.cpp
    modules/pool.cpp
)

target_include_directories(Modules PRIVATE modules)

target_link_libraries(Modules Inventory)