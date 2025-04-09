cmake_minimum_required(VERSION 3.22)

if(DEFINED ENV{VCPKG_ROOT})
    message(STATUS "VCPKG is installed. VCPKG_ROOT: $ENV{VCPKG_ROOT}")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
else()
    message(STATUS "VCPKG is not installed or VCPKG_ROOT is not set. Fetching vcpkg...")

    include(FetchContent)
    FetchContent_Declare(vcpkg GIT_REPOSITORY https://github.com/microsoft/vcpkg/ GIT_TAG 2024.09.30)

    FetchContent_MakeAvailable(vcpkg)

    set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
endif()

if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet" FORCE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

get_filename_component(SRC_FOLDER ${CMAKE_SOURCE_DIR}/../ ABSOLUTE)
get_filename_component(CONFIG_FOLDER ${CMAKE_SOURCE_DIR}/../etc/config/ ABSOLUTE)
get_filename_component(WINDOWS_FOLDER ${CMAKE_SOURCE_DIR}/../packages/windows/ ABSOLUTE)

project(wazuh-agent)

if(UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

find_package(OpenSSL REQUIRED)

include(cmake/CommonSettings.cmake)
set_common_settings()

if(ENABLE_LOGCOLLECTOR OR ENABLE_INVENTORY OR ENABLE_SCA)
    add_subdirectory(common)
else()
    add_subdirectory(common/config)
    add_subdirectory(common/data_provider)
    add_subdirectory(common/file_helper)
    add_subdirectory(common/logger)
    add_subdirectory(common/pal)
endif()

add_subdirectory(modules)
add_subdirectory(agent)

add_executable(wazuh-agent agent/src/main.cpp)
target_link_libraries(wazuh-agent PRIVATE Agent Config Logger OpenSSL::SSL OpenSSL::Crypto)

if(WIN32)
    include(cmake/SetWindowsManifest.cmake)
    set_windows_manifest()
endif()

include(cmake/ConfigureTarget.cmake)
configure_target(wazuh-agent)

include(cmake/config.cmake)
include(cmake/PrepareVersionFileForInstallation.cmake)

set(OUTPUT_CONFIG ${CONFIG_FOLDER}/wazuh-agent.yml)

# Platform specific config file
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SOURCE_CONFIG ${CONFIG_FOLDER}/wazuh_agent_linux.yml)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SOURCE_CONFIG ${CONFIG_FOLDER}/wazuh_agent_macos.yml)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SOURCE_CONFIG ${CONFIG_FOLDER}/wazuh_agent_windows.yml)
endif()

# Replace propper config in the generic placement
add_custom_command(OUTPUT ${OUTPUT_CONFIG} COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_CONFIG} ${OUTPUT_CONFIG}
                   DEPENDS ${SOURCE_CONFIG} COMMENT "Creating ${OUTPUT_CONFIG} from ${SOURCE_CONFIG}")

add_custom_target(generate_config ALL DEPENDS ${OUTPUT_CONFIG})

# Installation
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    install(DIRECTORY DESTINATION "${DATA_INSTALL_DIR}")

    install(CODE "file(MAKE_DIRECTORY \"${CONFIG_INSTALL_DIR}\")")
    install(FILES ${OUTPUT_CONFIG} DESTINATION ${CONFIG_INSTALL_DIR})
    install(FILES "${CMAKE_BINARY_DIR}/VERSION.json" DESTINATION ${PARENT_DIR})
    install(TARGETS wazuh-agent DESTINATION ${BIN_INSTALL_DIR})

    message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

    add_custom_target(
        generate_service_file ALL COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/configureServiceFile.cmake
        DEPENDS ${CMAKE_BINARY_DIR}/configureServiceFile.cmake)

    get_filename_component(SERVICE_FILENAME "${SERVICE_FILE}" NAME)
    configure_file(${CMAKE_SOURCE_DIR}/cmake/configureServiceFile.cmake.in
                   ${CMAKE_BINARY_DIR}/configureServiceFile.cmake @ONLY)
    install(SCRIPT ${CMAKE_BINARY_DIR}/configureServiceFile.cmake)
    install(FILES "${CMAKE_BINARY_DIR}/${SERVICE_FILENAME}" DESTINATION "${SERVICE_INSTALL_DIR}")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
    include(InstallRequiredSystemLibraries)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/$ENV{CUSTOM_CMAKE_CONFIG}/ DESTINATION . PATTERN "*-debug-symbols.zip"
                                                                                           EXCLUDE)
    install(FILES ${OUTPUT_CONFIG} DESTINATION .)
    install(FILES ${WINDOWS_FOLDER}/postinstall.ps1 DESTINATION .)
    install(FILES ${WINDOWS_FOLDER}/cleanup.ps1 DESTINATION .)
    install(FILES "${CMAKE_BINARY_DIR}/VERSION.json" DESTINATION .)

    set(CPACK_GENERATOR "WIX")
    set(CPACK_PACKAGE_NAME "Wazuh Agent")
    set(CPACK_PACKAGE_VERSION ${CLEAN_VERSION})

    if(DEFINED ENV{CUSTOM_PACKAGE_NAME})
        set(CPACK_PACKAGE_FILE_NAME "$ENV{CUSTOM_PACKAGE_NAME}")
    else()
        set(CPACK_PACKAGE_FILE_NAME "wazuh-agent-${CPACK_PACKAGE_VERSION}")
    endif()

    set(CPACK_PACKAGE_VENDOR "Wazuh, Inc.")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "Wazuh helps you to gain security visibility into your infrastructure "
        "by monitoring hosts at an operating system and application level. "
        "It provides capabilities like log analysis, file integrity monitoring, "
        "intrusion detection, and compliance monitoring.")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "wazuh-agent")
    set(CPACK_RESOURCE_FILE_LICENSE "${WINDOWS_FOLDER}/license.rtf")
    set(CPACK_WIX_UPGRADE_GUID "F495AC57-7BDE-4C4B-92D8-DBE40A9AA5A0")
    set(CPACK_WIX_UI_BANNER "${WINDOWS_FOLDER}/ui/bannrbmp.jpg")
    set(CPACK_WIX_UI_DIALOG "${WINDOWS_FOLDER}/ui/dlgbmp.jpg")
    set(CPACK_WIX_PRODUCT_ICON "${WINDOWS_FOLDER}/ui/favicon.ico")
    set(CPACK_WIX_PROPERTY_ARPNOREPAIR "yes")
    set(CPACK_WIX_PROPERTY_ALLUSERS "1")
    set(CPACK_WIX_PATCH_FILE "${WINDOWS_FOLDER}/wix-patch.xml")
    set(CPACK_WIX_EXTENSIONS "WixUtilExtension")
    include(CPack)
endif()

if(BUILD_TESTS)
    enable_testing()
endif()
