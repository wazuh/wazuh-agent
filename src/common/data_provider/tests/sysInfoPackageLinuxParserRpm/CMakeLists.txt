cmake_minimum_required(VERSION 3.22)

project(sysInfoPackageLinuxParserRPM_unit_test)

set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")

file(GLOB sysinfo_UNIT_TEST_SRC "*.cpp")
file(GLOB PARSER_SRC "${CMAKE_SOURCE_DIR}/src/packages/packageLinuxParserRpm.hpp")
file(GLOB RPM_SRC "${CMAKE_SOURCE_DIR}/src/packages/rpm*.cpp")

add_executable(sysInfoPackageLinuxParserRPM_unit_test
    ${sysinfo_UNIT_TEST_SRC}
    ${RPM_SRC}
    ${PARSER_SRC})

target_include_directories(sysInfoPackageLinuxParserRPM_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../file_helper/filesystem/tests/mocks)

set_property(TARGET sysInfoPackageLinuxParserRPM_unit_test PROPERTY COMPILE_FLAGS "-Wno-unused-parameter")

target_link_libraries(sysInfoPackageLinuxParserRPM_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME sysInfoPackageLinuxParserRPM_unit_test
         COMMAND sysInfoPackageLinuxParserRPM_unit_test)
