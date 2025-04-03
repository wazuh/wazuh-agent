cmake_minimum_required(VERSION 3.22)

project(dbsync_unit_test)

add_definitions(-DTEST_INPUTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

file(GLOB INTERFACE_UNITTEST_SRC "*.cpp")

add_executable(dbsync_unit_test ${INTERFACE_UNITTEST_SRC})

target_include_directories(dbsync_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sqlite)

configure_target(dbsync_unit_test)

target_link_libraries(
    dbsync_unit_test
    dbsync
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
    cjson)

add_test(NAME dbsync_unit_test COMMAND dbsync_unit_test)
