cmake_minimum_required(VERSION 3.22)

project(sysInfoPackages_unit_test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/packages)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../file_helper/filesystem/tests/mocks)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../file_helper/file_io/tests/mocks)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../jsonHelper/tests/mocks)

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

add_executable(sysInfoPackages_unit_test
    ${sysinfo_UNIT_TEST_SRC})

target_link_libraries(sysInfoPackages_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME sysInfoPackages_unit_test COMMAND sysInfoPackages_unit_test)
