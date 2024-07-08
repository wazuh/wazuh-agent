#include wrappers
include(${SRC_FOLDER}/unit_tests/wrappers/wazuh/shared/shared.cmake)

if(${TARGET} STREQUAL "winagent")
    link_directories(${SRC_FOLDER}/syscheckd/build/bin)
endif(${TARGET} STREQUAL "winagent")

# Tests list and flags
if(NOT ${TARGET} STREQUAL "winagent")
list(APPEND shared_tests_names "test_json_op")
list(APPEND shared_tests_flags "-Wl,--wrap,w_get_file_content -Wl,--wrap,cJSON_ParseWithOpts \
                                -Wl,--wrap,cJSON_PrintUnformatted -Wl,--wrap,fopen \
                                -Wl,--wrap,fwrite -Wl,--wrap,fclose -Wl,--wrap,wfopen \
                                -Wl,--wrap,fflush -Wl,--wrap,fgets \
                                -Wl,--wrap,fgetpos -Wl,--wrap,fgetc \
                                -Wl,--wrap,fread -Wl,--wrap,remove \
                                -Wl,--wrap,fseek -Wl,--wrap,popen ${DEBUG_OP_WRAPPERS}")
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
