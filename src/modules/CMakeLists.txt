cmake_minimum_required(VERSION 3.22)

project(ModuleManager)

include(../cmake/CommonSettings.cmake)
set_common_settings()

add_library(ModuleManager src/moduleManager.cpp)
target_include_directories(ModuleManager PUBLIC include)

message(STATUS "-------------------")
message(STATUS "Wazuh agent modules")
message(STATUS "-------------------")
if(ENABLE_MODULE_INVENTORY)
    message(STATUS "Inventory: enabled")
    target_compile_definitions(ModuleManager PRIVATE ENABLE_MODULE_INVENTORY)
    target_link_libraries(ModuleManager PRIVATE Inventory)
    add_subdirectory(inventory)
else()
    message(STATUS "Inventory: disabled")
endif()
message(STATUS "-------------------")

target_link_libraries(ModuleManager PUBLIC ConfigurationParser MultiTypeQueue Logger ModuleCommand)

include(../cmake/ConfigureTarget.cmake)
configure_target(ModuleManager)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
