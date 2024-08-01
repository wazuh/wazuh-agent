file(GLOB OS_REGEX_SOURCES src/*.c)

add_library(os_regex STATIC ${OS_REGEX_SOURCES})

target_include_directories(os_regex PUBLIC include)

target_link_libraries(os_regex utils time_op)