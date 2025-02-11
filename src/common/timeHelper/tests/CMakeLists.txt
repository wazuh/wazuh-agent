find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC
    "timeHelper_test.cpp"
    "main.cpp"
)

add_executable(time_helper_tests
    ${UTIL_CXX_UNITTEST_COMMON_SRC}
)

include(../../../cmake/ConfigureTarget.cmake)
configure_target(time_helper_tests)

target_include_directories(time_helper_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(time_helper_tests PRIVATE time_helper GTest::gtest)

add_test(NAME timeHelperTests COMMAND time_helper_tests)
