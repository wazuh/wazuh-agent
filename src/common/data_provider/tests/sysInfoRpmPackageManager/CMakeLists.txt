cmake_minimum_required(VERSION 3.22)

project(RpmPackageManager_unit_test)

file(GLOB RPM_UNIT_TEST_SRC "*.cpp")
file(GLOB RPM_SRC "${CMAKE_SOURCE_DIR}/src/packages/rpm*.cpp")

add_executable(Rpm_unit_test ${RPM_UNIT_TEST_SRC} ${RPM_SRC})

target_link_libraries(Rpm_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME Rpm_unit_test
         COMMAND Rpm_unit_test)
