cmake_minimum_required(VERSION 3.22)

project(inventory_test_tool)

file(GLOB INVENTORY_TESTTOOL_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        )

add_executable(inventory_test_tool
               ${INVENTORY_TESTTOOL_SRC}
               )

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(inventory_test_tool
        dbsync
        sysinfo
        Inventory
        psapi
        iphlpapi
        ws2_32
        crypt32
    )
else()
    target_link_libraries(inventory_test_tool
        dbsync
        sysinfo
        Inventory
        pthread
        dl
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

include(../../../cmake/ConfigureTarget.cmake)
configure_target(inventory_test_tool)
