#include wrappers
include(${SRC_FOLDER}/unit_tests/wrappers/wazuh/shared/shared.cmake)

if(${TARGET} STREQUAL "winagent")
    link_directories(${SRC_FOLDER}/syscheckd/build/bin)
endif(${TARGET} STREQUAL "winagent")

# Tests list and flags
list(APPEND shared_tests_names "test_validate_op")
set(VALIDATE_OP_FLAGS "-Wl,--wrap,w_expression_match -Wl,--wrap,w_calloc_expression_t \
                       -Wl,--wrap,w_expression_compile -Wl,--wrap,w_free_expression_t \
                       -Wl,--wrap,get_ipv4_numeric -Wl,--wrap,get_ipv6_numeric")
if(${TARGET} STREQUAL "winagent")
list(APPEND shared_tests_flags "${VALIDATE_OP_FLAGS} -Wl,--wrap,syscom_dispatch -Wl,--wrap,Start_win32_Syscheck \
                                -Wl,--wrap=is_fim_shutdown -Wl,--wrap=_imp__dbsync_initialize \
                                -Wl,--wrap=_imp__rsync_initialize -Wl,--wrap=fim_db_teardown ${DEBUG_OP_WRAPPERS}")
else()
list(APPEND shared_tests_flags "${VALIDATE_OP_FLAGS}")
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
