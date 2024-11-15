find_package(GTest CONFIG REQUIRED)

add_executable(ConfigurationParser_test configuration_parser_test.cpp)
configure_target(ConfigurationParser_test)
target_link_libraries(ConfigurationParser_test PUBLIC ConfigurationParser Config GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main Logger)
add_test(NAME ConfigParserTest COMMAND ConfigurationParser_test)
