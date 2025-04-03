find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC "jsonIO_test.cpp" "main.cpp")

add_executable(json_helper_tests ${UTIL_CXX_UNITTEST_COMMON_SRC})

include(../../../cmake/ConfigureTarget.cmake)
configure_target(json_helper_tests)

target_include_directories(json_helper_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(json_helper_tests PRIVATE json_helper GTest::gtest)

add_test(NAME jsonHelperTests COMMAND json_helper_tests)
