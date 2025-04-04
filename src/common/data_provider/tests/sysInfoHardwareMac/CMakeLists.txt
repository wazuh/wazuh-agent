cmake_minimum_required(VERSION 3.22)

project(sysInfoHardwareMac_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC "sysInfoHardwareMac_test.cpp" "sysInfoHardwareWrapperMac_test.cpp" "main.cpp")

add_executable(sysInfoHardwareMac_unit_test ${sysinfo_UNIT_TEST_SRC})

target_include_directories(sysInfoHardwareMac_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src
                                                                ${CMAKE_CURRENT_SOURCE_DIR}/../../src/hardware)

configure_target(sysInfoHardwareMac_unit_test)

target_link_libraries(sysInfoHardwareMac_unit_test PRIVATE sysinfo GTest::gtest GTest::gmock GTest::gtest_main
                                                           GTest::gmock_main)

add_test(NAME sysInfoHardwareMac_unit_test COMMAND sysInfoHardwareMac_unit_test)

if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm64.*|ARM64.*")
    file(GLOB sysinfo_ARM_UNIT_TEST_SRC "sysInfoHardwareWrapperARMMac_test.cpp" "main.cpp")

    add_executable(sysInfoHardwareARMMac_unit_test ${sysinfo_ARM_UNIT_TEST_SRC})

    target_include_directories(sysInfoHardwareARMMac_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src
                                                                       ${CMAKE_CURRENT_SOURCE_DIR}/../../src/hardware)

    configure_target(sysInfoHardwareARMMac_unit_test)

    target_link_libraries(sysInfoHardwareARMMac_unit_test PRIVATE sysinfo GTest::gtest GTest::gmock GTest::gtest_main
                                                                  GTest::gmock_main)

    add_test(NAME sysInfoHardwareARMMac_unit_test COMMAND sysInfoHardwareARMMac_unit_test)

else()
    file(GLOB sysinfo_X86_64_UNIT_TEST_SRC "sysInfoHardwareWrapperX86_64Mac_test.cpp" "main.cpp")

    add_executable(sysInfoHardwareX86_64Mac_unit_test ${sysinfo_X86_64_UNIT_TEST_SRC})

    target_include_directories(
        sysInfoHardwareX86_64Mac_unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/../../src/hardware)

    configure_target(sysInfoHardwareX86_64Mac_unit_test)

    target_link_libraries(sysInfoHardwareX86_64Mac_unit_test PRIVATE sysinfo GTest::gtest GTest::gmock
                                                                     GTest::gtest_main GTest::gmock_main)

    add_test(NAME sysInfoHardwareX86_64Mac_unit_test COMMAND sysInfoHardwareX86_64Mac_unit_test)
endif()
