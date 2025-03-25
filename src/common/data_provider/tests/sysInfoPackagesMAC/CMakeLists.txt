cmake_minimum_required(VERSION 3.22)

project(sysInfoMacPackage_unit_test)

file(COPY input_files DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

add_executable(sysInfoMacPackage_unit_test
    ${sysinfo_UNIT_TEST_SRC})

target_include_directories(sysInfoMacPackage_unit_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/packages
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../sqliteWrapper/tests/mocks
)

configure_target(sysInfoMacPackage_unit_test)

target_link_libraries(sysInfoMacPackage_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME sysInfoMacPackage_unit_test
         COMMAND sysInfoMacPackage_unit_test)
