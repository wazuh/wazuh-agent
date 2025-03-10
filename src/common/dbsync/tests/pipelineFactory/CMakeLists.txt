cmake_minimum_required(VERSION 3.22)

project(dbsyncPipelineFactory_unit_test)

set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")

include_directories(${CMAKE_SOURCE_DIR}/src/)
link_directories(${CMAKE_BINARY_DIR}/lib)


file(GLOB PIPELINE_FACTORY_UNITTEST_SRC
    "*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/)
link_directories(${CMAKE_BINARY_DIR}/lib)

file(GLOB PIPELINE_FACTORY_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/sqlite/*.cpp")

add_executable(dbsyncPipelineFactory_unit_test
    ${PIPELINE_FACTORY_UNITTEST_SRC}
    ${PIPELINE_FACTORY_SRC})
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(dbsyncPipelineFactory_unit_test
        dbsync
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        unofficial::sqlite3::sqlite3
        cjson
        utils
    )
else()
    target_link_libraries(dbsyncPipelineFactory_unit_test
        dbsync
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        pthread
        unofficial::sqlite3::sqlite3
        cjson
        dl
        utils
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_test(NAME dbsyncPipelineFactory_unit_test
         COMMAND dbsyncPipelineFactory_unit_test)
