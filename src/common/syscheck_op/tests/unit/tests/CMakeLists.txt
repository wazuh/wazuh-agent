#include wrappers
include(${SRC_FOLDER}/unit_tests/wrappers/wazuh/shared/shared.cmake)

if(${TARGET} STREQUAL "winagent")
    link_directories(${SRC_FOLDER}/syscheckd/build/bin)
endif(${TARGET} STREQUAL "winagent")

# Tests list and flags
list(APPEND shared_tests_names "test_syscheck_op")
set(SYSCHECK_OP_BASE_FLAGS "-Wl,--wrap,rmdir_ex -Wl,--wrap,wreaddir -Wl,--wrap,getpwuid_r -Wl,--wrap,w_getgrgid \
                            -Wl,--wrap,wstr_split -Wl,--wrap,OS_ConnectUnixDomain -Wl,--wrap,OS_SendSecureTCP \
                            -Wl,--wrap,sysconf -Wl,--wrap,win_strerror -Wl,--wrap,wfopen ${DEBUG_OP_WRAPPERS}")
if(${TARGET} STREQUAL "winagent")
    # cJSON_CreateArray@0 instead of cJSON_CreateArray since linker will be looking for cdecl forma
    # More info at: (https://devblogs.microsoft.com/oldnewthing/20040108-00/?p=41163)
    list(APPEND shared_tests_flags "${SYSCHECK_OP_BASE_FLAGS} -Wl,--wrap=syscom_dispatch -Wl,--wrap=is_fim_shutdown \
                                    -Wl,--wrap,cJSON_CreateArray@0 -Wl,--wrap,cJSON_CreateObject@0 -Wl,--wrap=Start_win32_Syscheck")
else()
    list(APPEND shared_tests_flags "${SYSCHECK_OP_BASE_FLAGS} -Wl,--wrap=cJSON_CreateArray,--wrap=cJSON_CreateObject -Wl,--wrap,getpid")
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
