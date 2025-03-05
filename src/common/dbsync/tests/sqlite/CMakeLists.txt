cmake_minimum_required(VERSION 3.22)

project(sqlite_unit_test)

set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/sqlite/)
file(GLOB SQLITE_UNIT_TEST_SRC
    "*.cpp")

file(GLOB SQLITE_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/sqlite/sqlite_wrapper.cpp")

add_executable(sqlite_unit_test
    ${SQLITE_UNIT_TEST_SRC}
    ${SQLITE_SRC})
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(sqlite_unit_test
        dbsync
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        unofficial::sqlite3::sqlite3
        cjson
    )
else()
    target_link_libraries(sqlite_unit_test
        dbsync
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        pthread
        unofficial::sqlite3::sqlite3
        cjson
        dl
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_test(NAME sqlite_unit_test
         COMMAND sqlite_unit_test)
