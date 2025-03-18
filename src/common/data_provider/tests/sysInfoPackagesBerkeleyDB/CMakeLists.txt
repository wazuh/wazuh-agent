cmake_minimum_required(VERSION 3.22)

project(sysInfoPackagesBerkeleyDB_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

add_executable(sysInfoPackagesBerkeleyDB_unit_test
    ${sysinfo_UNIT_TEST_SRC})

configure_target(sysInfoPackagesBerkeleyDB_unit_test)

target_link_libraries(sysInfoPackagesBerkeleyDB_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME sysInfoPackagesBerkeleyDB_unit_test
         COMMAND sysInfoPackagesBerkeleyDB_unit_test)
