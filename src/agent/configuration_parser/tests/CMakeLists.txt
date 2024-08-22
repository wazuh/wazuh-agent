find_package(GTest CONFIG REQUIRED)

add_executable(ConfigurationParser_test configuration_parser_test.cpp)
target_include_directories(ConfigurationParser_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(ConfigurationParser_test PUBLIC ConfigurationParser GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME ConfigParserTest COMMAND ConfigurationParser_test)
