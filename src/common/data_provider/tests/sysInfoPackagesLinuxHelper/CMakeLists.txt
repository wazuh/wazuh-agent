cmake_minimum_required(VERSION 3.22)

project(sysInfoPackagesLinuxHelper_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

add_executable(sysInfoPackagesLinuxHelper_unit_test
    ${sysinfo_UNIT_TEST_SRC})

target_link_libraries(sysInfoPackagesLinuxHelper_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME sysInfoPackagesLinuxHelper_unit_test
         COMMAND sysInfoPackagesLinuxHelper_unit_test)
