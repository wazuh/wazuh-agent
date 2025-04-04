cmake_minimum_required(VERSION 3.22)

project(sysInfoPackageLinuxParserRPM_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC "*.cpp")

add_executable(sysInfoPackageLinuxParserRPM_unit_test ${sysinfo_UNIT_TEST_SRC})

target_include_directories(
    sysInfoPackageLinuxParserRPM_unit_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src ${CMAKE_CURRENT_SOURCE_DIR}/../../src/packages
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../file_helper/filesystem/tests/mocks)

configure_target(sysInfoPackageLinuxParserRPM_unit_test)

target_link_libraries(sysInfoPackageLinuxParserRPM_unit_test PRIVATE sysinfo GTest::gtest GTest::gmock
                                                                     GTest::gtest_main GTest::gmock_main)

add_test(NAME sysInfoPackageLinuxParserRPM_unit_test COMMAND sysInfoPackageLinuxParserRPM_unit_test)
