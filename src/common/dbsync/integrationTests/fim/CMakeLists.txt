cmake_minimum_required(VERSION 3.22)

project(fim_integration_test)

add_definitions(-DTEST_DB_INPUTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

file(GLOB INTERFACE_UNITTEST_SRC "*.cpp")

add_executable(fim_integration_test ${INTERFACE_UNITTEST_SRC})

target_include_directories(fim_integration_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src
                                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sqlite)

configure_target(fim_integration_test)

target_link_libraries(
    fim_integration_test
    dbsync
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
    cjson)

add_test(NAME fim_integration_test COMMAND fim_integration_test)
