find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC "byteArrayHelper_test.cpp" "main.cpp")

add_executable(byte_array_helper_tests ${UTIL_CXX_UNITTEST_COMMON_SRC})

configure_target(byte_array_helper_tests)

target_include_directories(byte_array_helper_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(byte_array_helper_tests PRIVATE byte_array_helper GTest::gtest)

add_test(NAME byteArrayHelperTests COMMAND byte_array_helper_tests)
