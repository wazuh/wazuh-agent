cmake_minimum_required(VERSION 3.22)

project(RpmPackageManager_unit_test)

file(GLOB RPM_UNIT_TEST_SRC "*.cpp")
file(GLOB RPM_SRC "${CMAKE_SOURCE_DIR}/src/packages/rpm*.cpp")

add_executable(sysInfoRpm_unit_test ${RPM_UNIT_TEST_SRC} ${RPM_SRC})

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
