cmake_minimum_required(VERSION 3.22)

project(inventory_test_tool)

add_executable(inventory_test_tool ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_include_directories(inventory_test_tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include/
                                                       ${CMAKE_CURRENT_SOURCE_DIR}/../src/)

if(WIN32)
    target_link_libraries(
        inventory_test_tool
        dbsync
        sysinfo
        Inventory
        psapi
        iphlpapi
        ws2_32
        crypt32)
else()
    target_link_libraries(inventory_test_tool dbsync sysinfo Inventory)
endif()

include(../../../cmake/ConfigureTarget.cmake)
configure_target(inventory_test_tool)
