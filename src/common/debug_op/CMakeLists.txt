add_library(debug_op STATIC src/debug_op.c)

find_package(cJSON CONFIG REQUIRED)

target_include_directories(debug_op PUBLIC include)

target_link_libraries(debug_op
    utils
    time_op
    file_op
    mem_op
    privsep_op
    pthreads_op
    error_messages
    os_net
    binaries_op
    regex_op
    randombytes
    bzip2_op
    cjson)