find_package(GTest CONFIG REQUIRED)

file(COPY input_files DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC
    "hashHelper_test.cpp"
    "main.cpp"
)

add_executable(hash_helper_tests
    ${UTIL_CXX_UNITTEST_COMMON_SRC}
)

configure_target(hash_helper_tests)

target_include_directories(hash_helper_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(hash_helper_tests PRIVATE hash_helper GTest::gtest)

add_test(NAME hashHelperTests COMMAND hash_helper_tests)
