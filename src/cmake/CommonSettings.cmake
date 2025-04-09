function(set_common_settings)
    set(CMAKE_CXX_STANDARD 20 PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(PARENT_DIR "usr/share/wazuh-agent" PARENT_SCOPE)
        set(BIN_INSTALL_DIR "usr/share/wazuh-agent/bin" PARENT_SCOPE)
        set(CONFIG_INSTALL_DIR "etc/wazuh-agent" PARENT_SCOPE)
        set(SERVICE_INSTALL_DIR "/usr/lib/systemd/system" PARENT_SCOPE)
        set(DATA_INSTALL_DIR "var/lib/wazuh-agent" PARENT_SCOPE)
        set(RUN_INSTALL_DIR "var/run" PARENT_SCOPE)
        set(SERVICE_FILE "${CMAKE_SOURCE_DIR}/agent/service/wazuh-agent.service" PARENT_SCOPE)
        set(SHARED_CONFIG_INSTALL_DIR "${CONFIG_INSTALL_DIR}/shared" PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(PARENT_DIR "Library/Application Support/Wazuh agent.app" PARENT_SCOPE)
        set(BIN_INSTALL_DIR "Library/Application Support/Wazuh agent.app/bin" PARENT_SCOPE)
        set(CONFIG_INSTALL_DIR "Library/Application Support/Wazuh agent.app/etc" PARENT_SCOPE)
        set(SERVICE_INSTALL_DIR "/Library/LaunchDaemons" PARENT_SCOPE)
        set(DATA_INSTALL_DIR "Library/Application Support/Wazuh agent.app/var" PARENT_SCOPE)
        set(RUN_INSTALL_DIR "private/var/run" PARENT_SCOPE)
        set(SERVICE_FILE "${CMAKE_SOURCE_DIR}/agent/service/com.wazuh.agent.plist" PARENT_SCOPE)
        set(SHARED_CONFIG_INSTALL_DIR "${CONFIG_INSTALL_DIR}/shared" PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # We need to define NOMINMAX to avoid conflicts with std::min and std::max
        add_definitions(-DNOMINMAX)

        if(DEFINED ENV{ProgramFiles})
            set(BIN_INSTALL_DIR "$ENV{ProgramFiles}\\wazuh-agent" PARENT_SCOPE)
        else()
            set(BIN_INSTALL_DIR "C:\\Program Files\\wazuh-agent" PARENT_SCOPE)
        endif()

        if(DEFINED ENV{ProgramData})
            set(CONFIG_INSTALL_DIR "$ENV{ProgramData}\\wazuh-agent\\config" PARENT_SCOPE)
            set(DATA_INSTALL_DIR "$ENV{ProgramData}\\wazuh-agent\\var" PARENT_SCOPE)
            set(RUN_INSTALL_DIR "$ENV{ProgramData}\\wazuh-agent\\run" PARENT_SCOPE)
            set(SHARED_CONFIG_INSTALL_DIR "$ENV{ProgramData}\\wazuh-agent\\config\\shared" PARENT_SCOPE)
        else()
            set(CONFIG_INSTALL_DIR "C:\\ProgramData\\wazuh-agent\\config" PARENT_SCOPE)
            set(DATA_INSTALL_DIR "C:\\ProgramData\\wazuh-agent\\var" PARENT_SCOPE)
            set(RUN_INSTALL_DIR "C:\\ProgramData\\wazuh-agent\\run" PARENT_SCOPE)
            set(SHARED_CONFIG_INSTALL_DIR "C:\\ProgramData\\wazuh-agent\\config\\shared" PARENT_SCOPE)
        endif()
    else()
        message(FATAL_ERROR "Not supported OS")
    endif()

    option(BUILD_TESTS "Enable tests building" OFF)
    option(COVERAGE "Enable coverage report" OFF)
    option(ENABLE_INVENTORY "Enable Inventory module" ON)
    option(ENABLE_LOGCOLLECTOR "Enable Logcollector module" ON)
    option(ENABLE_SCA "Enable SCA module" ON)

    if(COVERAGE)
        if(NOT TARGET coverage)
            set(CMAKE_BUILD_TYPE Debug)

            if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                add_compile_options(-O0 -fprofile-arcs -ftest-coverage -fprofile-instr-generate -fcoverage-mapping)
                add_link_options(-fprofile-arcs -ftest-coverage -fprofile-instr-generate -fcoverage-mapping)
            else()
                add_compile_options(-O0 --coverage) # note that -g3 is required for gcov but it's already below
                add_link_options(--coverage)
            endif()

            # We found that gcc gcov doesn't report coverage on coroutines, so we use llvm-cov if available
            find_program(LLVM_COV_EXECUTABLE NAMES llvm-cov)

            if(LLVM_COV_EXECUTABLE)
                set(GCOV_EXECUTABLE "llvm-cov gcov")
            else()
                set(GCOV_EXECUTABLE "gcov")
            endif()

            add_custom_target(
                coverage
                COMMAND ${CMAKE_MAKE_PROGRAM} test
                COMMAND gcovr --gcov-executable "${GCOV_EXECUTABLE}" -v -r .. -e '.*main\.cpp' -e '.*vcpkg.*' -e
                        '.*mock_.*' -e '.*_test.*' --html --html-details -o coverage.html
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report")
        endif()
    endif()

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "RelWithDebInfo" PARENT_SCOPE)
    endif()

    if(MSVC)
        set(CMAKE_CXX_FLAGS "/Zi /DWIN32 /EHsc" PARENT_SCOPE)
        add_compile_options(/Zc:preprocessor)
    else()
        set(CMAKE_CXX_FLAGS "-g3" PARENT_SCOPE)
    endif()

    if(APPLE)
        add_link_options("-Wl,-no_warn_duplicate_libraries")
    endif()

    if(ENABLE_INVENTORY)
        add_definitions(-DENABLE_INVENTORY)
    endif()

    if(ENABLE_LOGCOLLECTOR)
        add_definitions(-DENABLE_LOGCOLLECTOR)
    endif()

    if(ENABLE_SCA)
        add_definitions(-DENABLE_SCA)
    endif()
endfunction()
