find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC
    "cmdHelper_test.cpp"
    "main.cpp"
)

add_executable(cmd_helper_tests
    ${UTIL_CXX_UNITTEST_COMMON_SRC}
)

configure_target(cmd_helper_tests)

target_include_directories(cmd_helper_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(cmd_helper_tests PRIVATE cmd_helper GTest::gtest)

add_test(NAME cmdHelperTests COMMAND cmd_helper_tests)
