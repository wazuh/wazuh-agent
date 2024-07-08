#include wrappers
include(${SRC_FOLDER}/unit_tests/wrappers/wazuh/shared/shared.cmake)

if(${TARGET} STREQUAL "winagent")
    link_directories(${SRC_FOLDER}/syscheckd/build/bin)
endif(${TARGET} STREQUAL "winagent")

# Tests list and flags
list(APPEND shared_tests_names "test_file_op")
if(NOT ${TARGET} STREQUAL "winagent")
    set(FILE_OP_BASE_FLAGS "-Wl,--wrap,stat,--wrap,chmod,--wrap,getpid,--wrap,wfopen \
                            -Wl,--wrap,unlink,--wrap,fopen,--wrap,fflush,--wrap,fclose \
                            -Wl,--wrap,fread,--wrap,ftell,--wrap,fseek,--wrap,fwrite,--wrap,remove \
                            -Wl,--wrap,fprintf,--wrap,fgets,--wrap,File_DateofChange \
                            -Wl,--wrap,bzip2_uncompress,--wrap,lstat,--wrap,popen \
                            -Wl,--wrap,gzopen,--wrap,gzread,--wrap,gzclose,--wrap,fgetc \
                            -Wl,--wrap,gzeof,--wrap,gzerror,--wrap,gzwrite,--wrap,fgetpos \
                            -Wl,--wrap,realpath,--wrap,getenv,--wrap,atexit ${DEBUG_OP_WRAPPERS} \
                            -Wl,--wrap,merror,--wrap,fseek")
    list(APPEND shared_tests_flags "${FILE_OP_BASE_FLAGS}")
else()
    list(APPEND shared_tests_flags "-Wl,--wrap,get_windows_file_time_epoch,--wrap,mdebug2 \
                                    -Wl,--wrap,syscom_dispatch -Wl,--wrap,Start_win32_Syscheck \
                                    -Wl,--wrap=is_fim_shutdown -Wl,--wrap=_imp__dbsync_initialize \
                                    -Wl,--wrap=_imp__rsync_initialize -Wl,--wrap=fim_db_teardown ${DEBUG_OP_WRAPPERS}")
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
