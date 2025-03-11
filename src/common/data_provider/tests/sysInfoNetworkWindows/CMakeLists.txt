cmake_minimum_required(VERSION 3.22)

project(sysInfoNetworkWindows_unit_test)

set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

file(GLOB SYSINFO_SRC
    "${CMAKE_SOURCE_DIR}/src/network/networkInterfaceWindows.cpp")

find_package(GTest CONFIG REQUIRED)

add_executable(sysInfoNetworkWindows_unit_test
    ${sysinfo_UNIT_TEST_SRC}
    ${SYSINFO_SRC})

target_link_libraries(sysInfoNetworkWindows_unit_test
    sysinfo
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
    cjson
)

add_test(NAME sysInfoNetworkWindows_unit_test
         COMMAND sysInfoNetworkWindows_unit_test)
