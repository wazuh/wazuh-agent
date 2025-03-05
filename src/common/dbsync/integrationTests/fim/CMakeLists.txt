cmake_minimum_required(VERSION 3.22)

project(fim_integration_test)

set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")

add_definitions(-DTEST_DB_INPUTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/)
link_directories(${CMAKE_BINARY_DIR}/lib)

file(GLOB INTERFACE_UNITTEST_SRC
    "*.cpp")

file(GLOB DBSYNC_IMP_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/sqlite/*.cpp")

add_executable(fim_integration_test
    ${INTERFACE_UNITTEST_SRC}
    ${DBSYNC_IMP_SRC})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(fim_integration_test
        dbsync
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        cjson
        unofficial::sqlite3::sqlite3
        deleter_helper
    )
else()
    target_link_libraries(fim_integration_test
        dbsync
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        cjson
        unofficial::sqlite3::sqlite3
        pthread
        dl
        deleter_helper
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_test(NAME fim_integration_test
         COMMAND fim_integration_test)
