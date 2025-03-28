cmake_minimum_required(VERSION 3.22)

project(sysinfo_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC
    "sysInfo_test.cpp"
    "main.cpp")

if(WIN32)
    list(APPEND sysinfo_UNIT_TEST_SRC
    "sysOsInfo_test.cpp")
else()
    list(APPEND sysinfo_UNIT_TEST_SRC
    "sysInfoParsers_test.cpp")
endif()

add_executable(sysinfo_unit_test
    ${sysinfo_UNIT_TEST_SRC})

target_include_directories(sysinfo_unit_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/osinfo
)

configure_target(sysinfo_unit_test)

target_link_libraries(sysinfo_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
    cjson
)

add_test(NAME sysinfo_unit_test
         COMMAND sysinfo_unit_test)
