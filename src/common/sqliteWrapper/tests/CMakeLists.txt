find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC
    "sqliteWrapper_test.cpp"
    "main.cpp"
)

add_executable(sqlite_wrapper_tests
    ${UTIL_CXX_UNITTEST_COMMON_SRC}
)

configure_target(sqlite_wrapper_tests)

target_include_directories(sqlite_wrapper_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(sqlite_wrapper_tests PRIVATE sqlite_wrapper GTest::gtest GTest::gmock)

add_test(NAME sqliteWrapperTests COMMAND sqlite_wrapper_tests)
