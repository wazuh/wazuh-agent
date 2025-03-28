cmake_minimum_required(VERSION 3.22)

project(dbengine_unit_test)

file(GLOB DBENGINE_UNITTEST_SRC
    "*.cpp")

add_executable(dbengine_unit_test
    ${DBENGINE_UNITTEST_SRC})

target_include_directories(dbengine_unit_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sqlite
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../sqliteWrapper/tests/mocks
)

configure_target(dbengine_unit_test)

target_link_libraries(dbengine_unit_test
    dbsync
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME dbengine_unit_test
         COMMAND dbengine_unit_test)
