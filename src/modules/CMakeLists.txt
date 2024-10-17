cmake_minimum_required(VERSION 3.22)

project(ModuleManager)

include(../cmake/CommonSettings.cmake)
set_common_settings()

add_subdirectory(centralized_configuration)
add_subdirectory(inventory)

if (ENABLE_LOGCOLLECTOR)
    add_subdirectory(logcollector)
endif()

add_library(ModuleManager src/moduleManager.cpp)
target_include_directories(ModuleManager PUBLIC include)
target_link_libraries(ModuleManager PUBLIC Inventory)

if (ENABLE_LOGCOLLECTOR)
    target_link_libraries(ModuleManager PUBLIC Logcollector)
endif()

include(../cmake/ConfigureTarget.cmake)
configure_target(ModuleManager)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
