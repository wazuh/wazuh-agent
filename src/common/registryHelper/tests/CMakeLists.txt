find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_WINDOWS_SRC "registryHelper_test.cpp")

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC "main.cpp")

add_executable(registry_helper_tests ${UTIL_CXX_UNITTEST_COMMON_SRC} ${UTIL_CXX_UNITTEST_WINDOWS_SRC})

configure_target(registry_helper_tests)

target_include_directories(registry_helper_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(registry_helper_tests PRIVATE registry_helper GTest::gtest)

add_test(NAME registryHelperTests COMMAND registry_helper_tests)
