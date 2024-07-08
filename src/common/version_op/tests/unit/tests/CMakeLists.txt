#include wrappers
include(${SRC_FOLDER}/unit_tests/wrappers/wazuh/shared/shared.cmake)

if(${TARGET} STREQUAL "winagent")
    link_directories(${SRC_FOLDER}/syscheckd/build/bin)
endif(${TARGET} STREQUAL "winagent")

# Tests list and flags
list(APPEND shared_tests_names "test_version_op")
set(VERSION_OP_BASE_FLAGS "-Wl,--wrap,fopen -Wl,--wrap,fclose -Wl,--wrap,fflush -Wl,--wrap,fread -Wl,--wrap,wfopen \
                           -Wl,--wrap,fseek -Wl,--wrap,fwrite -Wl,--wrap,remove -Wl,--wrap,fprintf -Wl,--wrap,fgets \
                           -Wl,--wrap,fgetpos -Wl,--wrap,fgetc -Wl,--wrap,popen -Wl,--wrap,pclose \
                           -Wl,--wrap,get_binary_path")
if(${TARGET} STREQUAL "winagent")
list(APPEND shared_tests_flags "${VERSION_OP_BASE_FLAGS} -Wl,--wrap,syscom_dispatch -Wl,--wrap,Start_win32_Syscheck \
                                -Wl,--wrap=is_fim_shutdown -Wl,--wrap=_imp__dbsync_initialize \
                                -Wl,--wrap=_imp__rsync_initialize -Wl,--wrap=fim_db_teardown")
else()
list(APPEND shared_tests_flags "${VERSION_OP_BASE_FLAGS}")
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
