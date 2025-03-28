cmake_minimum_required(VERSION 3.22)

project(RpmPackageManager_unit_test)

file(GLOB RPM_UNIT_TEST_SRC
    "*.cpp")

add_executable(sysInfoRpm_unit_test ${RPM_UNIT_TEST_SRC})

target_include_directories(sysInfoRpm_unit_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/packages
)

configure_target(sysInfoRpm_unit_test)

target_link_libraries(sysInfoRpm_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME sysInfoRpm_unit_test
         COMMAND sysInfoRpm_unit_test)
