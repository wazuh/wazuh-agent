find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC "pipelineNodes_test.cpp" "main.cpp")

add_executable(pipeline_helper_tests ${UTIL_CXX_UNITTEST_COMMON_SRC})

include(../../../cmake/ConfigureTarget.cmake)
configure_target(pipeline_helper_tests)

target_include_directories(pipeline_helper_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(pipeline_helper_tests PRIVATE pipeline_helper GTest::gtest GTest::gmock)

add_test(NAME pipelineHelperTests COMMAND pipeline_helper_tests)
