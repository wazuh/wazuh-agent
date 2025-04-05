cmake_minimum_required(VERSION 3.22)

project(sysInfoWindows_unit_test)

file(GLOB UTILS_SYSYINFO_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src/utilsWrapperWin.cpp")

file(GLOB sysinfo_UNIT_TEST_SRC "*.cpp")

add_executable(sysInfoWindows_unit_test
    ${sysinfo_UNIT_TEST_SRC}
    ${UTILS_SYSYINFO_SRC}
)

target_include_directories(sysInfoWindows_unit_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/osinfo
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/packages
)

configure_target(sysInfoWindows_unit_test)

target_link_libraries(sysInfoWindows_unit_test PRIVATE
    wbemuuid
    wuguid
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME sysInfoWindows_unit_test COMMAND sysInfoWindows_unit_test)