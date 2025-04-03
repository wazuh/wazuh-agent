cmake_minimum_required(VERSION 3.22)

project(sysInfoWindows_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC "*.cpp")

add_executable(sysInfoWindows_unit_test ${sysinfo_UNIT_TEST_SRC})

target_include_directories(sysInfoWindows_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src
                                                            ${CMAKE_CURRENT_SOURCE_DIR}/../../src/osinfo)

configure_target(sysInfoWindows_unit_test)

target_link_libraries(sysInfoWindows_unit_test PRIVATE sysinfo GTest::gtest GTest::gmock GTest::gtest_main
                                                       GTest::gmock_main)

add_test(NAME sysInfoWindows_unit_test COMMAND sysInfoWindows_unit_test)
