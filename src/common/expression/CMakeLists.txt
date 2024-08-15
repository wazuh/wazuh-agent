find_package(pcre2 CONFIG REQUIRED)

add_library(expression STATIC src/expression.c)

target_include_directories(expression PUBLIC include)

target_link_libraries(expression utils debug_op PCRE2::8BIT)