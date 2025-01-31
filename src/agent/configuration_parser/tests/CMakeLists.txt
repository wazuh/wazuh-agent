find_package(GTest CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_executable(ConfigurationParser_test configuration_parser_test.cpp)
configure_target(ConfigurationParser_test)
target_link_libraries(ConfigurationParser_test PUBLIC ConfigurationParser Config GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME ConfigParserTest COMMAND ConfigurationParser_test)

add_executable(ConfigurationParserUtils_test configuration_parser_utils_test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/configuration_parser_utils.cpp)
configure_target(ConfigurationParserUtils_test)
target_include_directories(ConfigurationParserUtils_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(ConfigurationParserUtils_test PUBLIC GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main yaml-cpp::yaml-cpp)
add_test(NAME ConfigParserUtilsTest COMMAND ConfigurationParserUtils_test)
