cmake_minimum_required(VERSION 3.22)

project(sysInfoPort_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC "*.cpp")

add_executable(sysInfoPort_unit_test ${sysinfo_UNIT_TEST_SRC})

target_include_directories(sysInfoPort_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src
                                                         ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ports)

configure_target(sysInfoPort_unit_test)

target_link_libraries(sysInfoPort_unit_test PRIVATE sysinfo GTest::gtest GTest::gmock GTest::gtest_main
                                                    GTest::gmock_main)

add_test(NAME sysInfoPort_unit_test COMMAND sysInfoPort_unit_test)
