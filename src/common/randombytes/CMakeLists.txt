add_library(randombytes STATIC src/randombytes.c)

target_include_directories(randombytes PUBLIC include)

target_link_libraries(randombytes
    utils
    Logger
    time_op)