find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_WINDOWS_SRC
    "windowsHelper_test.cpp"
)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC
    "main.cpp"
)

add_executable(windows_helper_tests
    ${UTIL_CXX_UNITTEST_COMMON_SRC}
    ${UTIL_CXX_UNITTEST_WINDOWS_SRC}
)

configure_target(windows_helper_tests)

target_include_directories(windows_helper_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(windows_helper_tests PRIVATE windows_helper GTest::gtest)

add_test(NAME windowsHelperTests COMMAND windows_helper_tests)
