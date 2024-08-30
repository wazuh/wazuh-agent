cmake_minimum_required(VERSION 3.22)

project(ModuleManager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(inventory)

add_library(ModuleManager src/moduleManager.cpp)
target_include_directories(ModuleManager PUBLIC include)
target_link_libraries(ModuleManager PUBLIC inventory)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
