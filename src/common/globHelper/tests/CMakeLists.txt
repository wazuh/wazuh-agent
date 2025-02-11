find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC
    "globHelper_test.cpp"
    "main.cpp"
)

add_executable(glob_helper_tests
    ${UTIL_CXX_UNITTEST_COMMON_SRC}
)

include(../../../cmake/ConfigureTarget.cmake)
configure_target(glob_helper_tests)

target_include_directories(glob_helper_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(glob_helper_tests PRIVATE glob_helper GTest::gtest)

add_test(NAME globHelperTests COMMAND glob_helper_tests)
