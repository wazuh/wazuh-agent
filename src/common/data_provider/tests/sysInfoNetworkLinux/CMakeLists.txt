cmake_minimum_required(VERSION 3.22)

project(sysInfoNetworkLinux_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

file(GLOB SYSINFO_SRC
    "${CMAKE_SOURCE_DIR}/src/network/networkInterfaceLinux.cpp")

add_executable(sysInfoNetworkLinux_unit_test
    ${sysinfo_UNIT_TEST_SRC}
    ${SYSINFO_SRC})

target_link_libraries(sysInfoNetworkLinux_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME sysInfoNetworkLinux_unit_test
         COMMAND sysInfoNetworkLinux_unit_test)
