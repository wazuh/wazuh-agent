cmake_minimum_required(VERSION 3.22)

project(sysInfoPackages_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC "*.cpp")

add_executable(sysInfoPackages_unit_test ${sysinfo_UNIT_TEST_SRC})

target_include_directories(
    sysInfoPackages_unit_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src
            ${CMAKE_CURRENT_SOURCE_DIR}/../../src/packages
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../file_helper/filesystem/tests/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../file_helper/file_io/tests/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../jsonHelper/tests/mocks)

configure_target(sysInfoPackages_unit_test)

target_link_libraries(sysInfoPackages_unit_test PRIVATE sysinfo GTest::gtest GTest::gmock GTest::gtest_main
                                                        GTest::gmock_main)

add_test(NAME sysInfoPackages_unit_test COMMAND sysInfoPackages_unit_test)
