cmake_minimum_required(VERSION 3.22)

project(unit_tests)

find_package(GTest CONFIG REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(sysInfoPackagesLinuxHelper)
    add_subdirectory(sysInfoPackagesBerkeleyDB)
    add_subdirectory(sysInfoNetworkLinux)
    add_subdirectory(sysInfoRpmPackageManager)
    add_subdirectory(sysInfoPackageLinuxParserRpm)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(sysInfoHardwareMac)
    add_subdirectory(sysInfoNetworkBSD)
    add_subdirectory(sysInfoPackagesMAC)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(sysInfoWin)
    add_subdirectory(sysInfoNetworkWindows)
endif()

add_subdirectory(sysInfo)
add_subdirectory(sysInfoPackages)
add_subdirectory(sysInfoPorts)
