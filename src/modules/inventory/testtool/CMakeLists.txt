cmake_minimum_required(VERSION 3.22)

project(inventory_test_tool)

if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)
add_definitions(-DWAZUH_UNIT_TESTING)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual -Wunused -Wcast-align -Wformat=2 -std=c++14 -pthread")
  if(FSANITIZE)
    set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address,leak,undefined")
  endif(FSANITIZE)
else()
  set(CMAKE_CXX_FLAGS "/Zi /W4 /permissive- /std:c++14 /MT")
endif()

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
        crypto
        ssl
        ws2_32
        crypt32
        -static-libstdc++
    )
else()
    target_link_libraries(inventory_test_tool
        dbsync
        sysinfo
        Inventory
        dl
        proc
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
