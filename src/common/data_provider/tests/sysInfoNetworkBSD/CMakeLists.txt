cmake_minimum_required(VERSION 3.22)

project(sysInfoNetworkBSD_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

file(GLOB SYSINFO_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/network/networkInterfaceBSD.cpp")

add_executable(sysInfoNetworkBSD_unit_test
    ${sysinfo_UNIT_TEST_SRC}
    ${SYSINFO_SRC})

target_link_libraries(sysInfoNetworkBSD_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)
add_test(NAME sysInfoNetworkBSD_unit_test
         COMMAND sysInfoNetworkBSD_unit_test)
