cmake_minimum_required(VERSION 3.22)

project(sysInfoNetworkLinux_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC "*.cpp")

add_executable(sysInfoNetworkLinux_unit_test ${sysinfo_UNIT_TEST_SRC})

target_include_directories(sysInfoNetworkLinux_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src
                                                                 ${CMAKE_CURRENT_SOURCE_DIR}/../../src/network)

configure_target(sysInfoNetworkLinux_unit_test)

target_link_libraries(sysInfoNetworkLinux_unit_test PRIVATE sysinfo GTest::gtest GTest::gmock GTest::gtest_main
                                                            GTest::gmock_main)

add_test(NAME sysInfoNetworkLinux_unit_test COMMAND sysInfoNetworkLinux_unit_test)
