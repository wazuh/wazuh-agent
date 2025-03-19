cmake_minimum_required(VERSION 3.22)

project(sysinfo_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC
    "*.cpp")

file(GLOB SYSINFO_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/sysInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/osinfo/sysOsParsers.cpp")

add_executable(sysinfo_unit_test
    ${sysinfo_UNIT_TEST_SRC}
    ${SYSINFO_SRC})

configure_target(sysinfo_unit_test)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(sysinfo_unit_test PRIVATE
        sysinfo
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        cjson
    )
else()
    target_link_libraries(sysinfo_unit_test PRIVATE
        sysinfo
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        pthread
        cjson
        dl
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_test(NAME sysinfo_unit_test
         COMMAND sysinfo_unit_test)
