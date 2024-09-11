cmake_minimum_required(VERSION 3.12.4)

project(dbengine_unit_test)


if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++14 --coverage")
else()
  set(CMAKE_CXX_FLAGS "/W4 /permissive- /std:c++14 /MT /coverage")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include/)
link_directories(${CMAKE_BINARY_DIR}/lib)


file(GLOB DBENGINE_UNITTEST_SRC
    "*.cpp")

file(GLOB SQLITE_ENGINE_SRC
    "${CMAKE_SOURCE_DIR}/src/sqlite/sqlite_dbengine.cpp")

add_executable(dbengine_unit_test 
    ${DBENGINE_UNITTEST_SRC} 
    ${SQLITE_ENGINE_SRC})
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(dbengine_unit_test
        debug gtestd
        debug gmockd
        debug gtest_maind
        debug gmock_maind
        optimized gtest
        optimized gmock
        optimized gtest_main
        optimized gmock_main
        pthread
        -static-libgcc -static-libstdc++
    )
else()
    target_link_libraries(dbengine_unit_test
        debug gtestd
        debug gmockd
        debug gtest_maind
        debug gmock_maind
        optimized gtest
        optimized gmock
        optimized gtest_main
        optimized gmock_main
        pthread
        dl
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_test(NAME dbengine_unit_test
         COMMAND dbengine_unit_test)
