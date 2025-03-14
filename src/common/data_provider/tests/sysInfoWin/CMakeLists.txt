cmake_minimum_required(VERSION 3.22)

project(sysInfoWindows_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

find_package(GTest CONFIG REQUIRED)

add_executable(sysInfoWindows_unit_test
    ${sysinfo_UNIT_TEST_SRC})

target_link_libraries(sysInfoWindows_unit_test
    sysinfo
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
    cjson
)

add_test(NAME sysInfoWindows_unit_test
         COMMAND sysInfoWindows_unit_test)
