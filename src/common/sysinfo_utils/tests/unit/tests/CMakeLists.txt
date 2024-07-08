#include wrappers
include(${SRC_FOLDER}/unit_tests/wrappers/wazuh/shared/shared.cmake)

if(${TARGET} STREQUAL "winagent")
    link_directories(${SRC_FOLDER}/syscheckd/build/bin)
endif(${TARGET} STREQUAL "winagent")

# Tests list and flags
list(APPEND shared_tests_names "test_sysinfo_utils")

if(${TARGET} STREQUAL "winagent")
    # cJSON_CreateArray@0 instead of cJSON_CreateArray since linker will be looking for cdecl forma
    # More info at: (https://devblogs.microsoft.com/oldnewthing/20040108-00/?p=41163)
    list(APPEND shared_tests_flags "-Wl,--wrap,so_free_library -Wl,--wrap,so_get_function_sym \
                                -Wl,--wrap,so_get_module_handle -Wl,--wrap,so_get_module_handle_on_path \
                                -Wl,--wrap,sysinfo_os -Wl,--wrap,sysinfo_processes -Wl,--wrap,sysinfo_free_result \
                                -Wl,--wrap,cJSON_GetObjectItem@8 -Wl,--wrap,cJSON_GetStringValue@4 \
                                -Wl,--wrap,syscom_dispatch -Wl,--wrap,Start_win32_Syscheck \
                                -Wl,--wrap=is_fim_shutdown -Wl,--wrap=_imp__dbsync_initialize \
                                -Wl,--wrap=_imp__rsync_initialize -Wl,--wrap=fim_db_teardown")
else()
    list(APPEND shared_tests_flags "-Wl,--wrap,so_free_library -Wl,--wrap,so_get_function_sym \
                                -Wl,--wrap,so_get_module_handle -Wl,--wrap,so_get_module_handle_on_path \
                                -Wl,--wrap,sysinfo_os -Wl,--wrap,sysinfo_processes -Wl,--wrap,sysinfo_free_result \
                                -Wl,--wrap,cJSON_GetObjectItem -Wl,--wrap,cJSON_GetStringValue")
endif()

# Compiling tests
list(LENGTH shared_tests_names count)
math(EXPR count "${count} - 1")
foreach(counter RANGE ${count})
    list(GET shared_tests_names ${counter} test_name)
    list(GET shared_tests_flags ${counter} test_flags)

    add_executable(${test_name} ${test_name}.c)

    if(${TARGET} STREQUAL "server")
        target_link_libraries(
            ${test_name}
            ${WAZUHLIB}
            ${WAZUHEXT}
            ANALYSISD_O
            ${TEST_DEPS}
        )
    else()
        target_link_libraries(
            ${test_name}
            ${TEST_DEPS}
        )
        if(${TARGET} STREQUAL "winagent")
          target_link_libraries(${test_name} fimdb)
        endif(${TARGET} STREQUAL "winagent")
    endif()

    if(NOT test_flags STREQUAL " ")
        target_link_libraries(
            ${test_name}
            ${test_flags}
        )
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
