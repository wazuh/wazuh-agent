cmake_minimum_required(VERSION 3.22)

project(inventory_test_tool)

if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -std=c++14")
  if(FSANITIZE)
    set(CMAKE_CXX_FLAGS_DEBUG "-pthread -fsanitize=address,leak,undefined")
  endif(FSANITIZE)
else()
  set(CMAKE_CXX_FLAGS "/Zi /W4 /permissive- /std:c++14 /MT /EHsc")
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
