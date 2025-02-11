cmake_minimum_required(VERSION 3.22)

project(sysInfoWindows_unit_test)

set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

    add_executable(sysInfoWindows_unit_test
    ${sysinfo_UNIT_TEST_SRC})

target_link_libraries(sysInfoWindows_unit_test
    sysinfo
    debug gtestd
    debug gmockd
    debug gtest_maind
    debug gmock_maind
    optimized gtest
    optimized gmock
    optimized gtest_main
    optimized gmock_main
    pthread
    sqlite3
    cjson
)

add_test(NAME sysInfoWindows_unit_test
         COMMAND sysInfoWindows_unit_test)
