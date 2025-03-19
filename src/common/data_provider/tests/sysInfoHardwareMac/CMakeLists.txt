cmake_minimum_required(VERSION 3.22)

project(sysInfoHardwareMac_unit_test)

file(GLOB sysinfo_UNIT_TEST_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/hardware/*X86_64Mac.cpp"
    "sysInfoHardwareMac_test.cpp"
    "sysInfoHardwareWrapperMac_test.cpp"
    "main.cpp")

add_executable(sysInfoHardwareMac_unit_test
    ${sysinfo_UNIT_TEST_SRC})

configure_target(sysInfoHardwareMac_unit_test)

target_link_libraries(sysInfoHardwareMac_unit_test PRIVATE
    sysinfo
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)

add_test(NAME sysInfoHardwareMac_unit_test
         COMMAND sysInfoHardwareMac_unit_test)

if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm64.*|ARM64.*")
    file(GLOB sysinfo_ARM_UNIT_TEST_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../src/hardware/*ARMMac.cpp"
        "sysInfoHardwareWrapperARMMac_test.cpp"
        "main.cpp")

    add_executable(sysInfoHardwareARMMac_unit_test
        ${sysinfo_ARM_UNIT_TEST_SRC})

    configure_target(sysInfoHardwareARMMac_unit_test)

    find_library(iokit_lib IOKit)
    if(NOT iokit_lib)
        message(FATAL_ERROR "IOKit library not found! Aborting...")
    endif()
    find_library(corefoundation_lib CoreFoundation)
    if(NOT corefoundation_lib)
        message(FATAL_ERROR "CoreFoundation library not found! Aborting...")
    endif()

    target_link_libraries(sysInfoHardwareARMMac_unit_test PRIVATE
        sysinfo
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
    )

    add_test(NAME sysInfoHardwareARMMac_unit_test
            COMMAND sysInfoHardwareARMMac_unit_test)
endif()
