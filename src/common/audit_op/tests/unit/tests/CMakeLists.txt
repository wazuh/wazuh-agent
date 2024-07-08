#include wrappers
include(${SRC_FOLDER}/unit_tests/wrappers/wazuh/shared/shared.cmake)

if(${TARGET} STREQUAL "winagent")
    link_directories(${SRC_FOLDER}/syscheckd/build/bin)
endif(${TARGET} STREQUAL "winagent")

# Tests list and flags
list(APPEND shared_tests_names "test_audit_op")
list(APPEND shared_tests_flags "-Wl,--wrap,audit_send -Wl,--wrap,popen \
                                -Wl,--wrap,select -Wl,--wrap,audit_get_reply -Wl,--wrap,wpopenv -Wl,--wrap,fgets \
                                -Wl,--wrap,wpclose -Wl,--wrap,audit_open -Wl,--wrap,audit_add_watch_dir \
                                -Wl,--wrap,audit_update_watch_perms -Wl,--wrap,audit_errno_to_name -Wl,--wrap,wfopen \
                                -Wl,--wrap,audit_rule_fieldpair_data -Wl,--wrap,fopen -Wl,--wrap,audit_add_rule_data \
                                -Wl,--wrap,audit_delete_rule_data -Wl,--wrap,audit_close -Wl,--wrap,fclose \
                                -Wl,--wrap,fflush -Wl,--wrap,fprintf -Wl,--wrap,fread -Wl,--wrap,fseek \
                                -Wl,--wrap,fwrite -Wl,--wrap,remove -Wl,--wrap,fgetpos -Wl,--wrap=fgetc \
                                -Wl,--wrap,get_binary_path ${DEBUG_OP_WRAPPERS}")

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
