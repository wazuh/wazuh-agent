cmake_minimum_required(VERSION 3.22)

project(inventory_test_tool)

if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)
add_definitions(-DWAZUH_UNIT_TESTING)

set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual -Wunused -Wcast-align -Wformat=2 -std=c++14 -pthread")

if(FSANITIZE)
  set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address,leak,undefined")
endif(FSANITIZE)

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
        inventory
        psapi
        iphlpapi
        crypto
        ssl
        ws2_32
        crypt32
        -static-libstdc++
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    target_link_libraries(inventory_test_tool
        dbsync
        sysinfo
        inventory
        pthread)
elseif (CMAKE_SYSTEM_NAME STREQUAL "AIX")
    target_link_libraries(inventory_test_tool
        dbsync
        sysinfo
        inventory
        dl)
else()
    target_link_libraries(inventory_test_tool
        dbsync
        sysinfo
        inventory
        dl
        proc
    )

    if(SOLARIS)
        target_link_libraries(inventory_test_tool
            nsl
            socket
        )
    endif(SOLARIS)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
