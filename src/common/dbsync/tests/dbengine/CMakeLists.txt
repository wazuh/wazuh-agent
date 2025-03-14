cmake_minimum_required(VERSION 3.22)

project(dbengine_unit_test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/ ${CMAKE_CURRENT_SOURCE_DIR}/../../../sqliteWrapper/tests/mocks)
link_directories(${CMAKE_BINARY_DIR}/lib)

file(GLOB DBENGINE_UNITTEST_SRC
    "*.cpp")

file(GLOB SQLITE_ENGINE_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/sqlite/*.cpp")

add_executable(dbengine_unit_test
    ${DBENGINE_UNITTEST_SRC}
    ${SQLITE_ENGINE_SRC})
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(dbengine_unit_test
        dbsync
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        cjson
    )
else()
    target_link_libraries(dbengine_unit_test
        dbsync
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        cjson
        pthread
        dl
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_test(NAME dbengine_unit_test
         COMMAND dbengine_unit_test)
