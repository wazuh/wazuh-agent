find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_WINDOWS_SRC "encodingWindows_test.cpp")

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC "main.cpp")

add_executable(encoding_helper_tests ${UTIL_CXX_UNITTEST_COMMON_SRC} ${UTIL_CXX_UNITTEST_WINDOWS_SRC})

configure_target(encoding_helper_tests)

target_include_directories(encoding_helper_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(encoding_helper_tests PRIVATE encoding_helper GTest::gtest)

add_test(NAME encodingHelperTests COMMAND encoding_helper_tests)
