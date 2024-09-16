cmake_minimum_required(VERSION 3.22)

project(ModuleManager)

include(../cmake/CommonSettings.cmake)
set_common_settings()

add_subdirectory(inventory)

add_library(ModuleManager src/moduleManager.cpp)
target_include_directories(ModuleManager PUBLIC include)
target_link_libraries(ModuleManager PUBLIC Inventory)

include(../cmake/ConfigureTarget.cmake)
configure_target(ModuleManager)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
