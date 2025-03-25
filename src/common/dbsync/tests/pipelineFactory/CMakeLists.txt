cmake_minimum_required(VERSION 3.22)

project(dbsyncPipelineFactory_unit_test)

file(GLOB PIPELINE_FACTORY_UNITTEST_SRC
    "*.cpp")

add_executable(dbsyncPipelineFactory_unit_test
    ${PIPELINE_FACTORY_UNITTEST_SRC})

target_include_directories(dbsyncPipelineFactory_unit_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sqlite
)

configure_target(dbsyncPipelineFactory_unit_test)

target_link_libraries(dbsyncPipelineFactory_unit_test
    dbsync
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME dbsyncPipelineFactory_unit_test
         COMMAND dbsyncPipelineFactory_unit_test)
