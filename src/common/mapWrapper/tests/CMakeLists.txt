find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC
    "mapWrapperSafe_test.cpp"
    "main.cpp"
)

add_executable(map_wrapper_tests
    ${UTIL_CXX_UNITTEST_COMMON_SRC}
)

include(../../../cmake/ConfigureTarget.cmake)
configure_target(map_wrapper_tests)

target_include_directories(map_wrapper_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(map_wrapper_tests PRIVATE map_wrapper GTest::gtest)

add_test(NAME mapWrapperTests COMMAND map_wrapper_tests)
