cmake_minimum_required(VERSION 3.22)

project(sysInfoNetworkWindows_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

add_executable(sysInfoNetworkWindows_unit_test
    ${sysinfo_UNIT_TEST_SRC})

target_include_directories(sysInfoNetworkWindows_unit_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/network
)

configure_target(sysInfoNetworkWindows_unit_test)

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
