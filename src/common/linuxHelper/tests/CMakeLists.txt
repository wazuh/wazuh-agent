find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC "main.cpp")

file(GLOB UTIL_CXX_UNITTEST_LINUX_SRC "linuxInfoHelper_test.cpp")

add_executable(linux_helper_tests ${UTIL_CXX_UNITTEST_COMMON_SRC} ${UTIL_CXX_UNITTEST_LINUX_SRC})

configure_target(linux_helper_tests)

target_include_directories(linux_helper_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(linux_helper_tests PRIVATE linux_helper cmd_helper GTest::gtest)

add_test(NAME linuxHelperTests COMMAND linux_helper_tests)
