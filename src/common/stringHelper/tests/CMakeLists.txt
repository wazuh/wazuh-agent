find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC
    "stringHelper_test.cpp"
    "main.cpp"
)

add_executable(string_helper_tests
    ${UTIL_CXX_UNITTEST_COMMON_SRC}
)

configure_target(string_helper_tests)

target_include_directories(string_helper_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(string_helper_tests PRIVATE string_helper GTest::gtest)

add_test(NAME stringHelperTests COMMAND string_helper_tests)
