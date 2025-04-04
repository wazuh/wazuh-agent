find_package(GTest CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_executable(ConfigurationParser_test configuration_parser_test.cpp)
configure_target(ConfigurationParser_test)
target_link_libraries(ConfigurationParser_test PUBLIC ConfigurationParser Config GTest::gtest GTest::gtest_main
                                                      GTest::gmock GTest::gmock_main)
add_test(NAME ConfigParserTest COMMAND ConfigurationParser_test)

add_executable(YamlUtils_test yaml_utils_test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/yaml_utils.cpp)
configure_target(YamlUtils_test)
target_include_directories(YamlUtils_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(YamlUtils_test PUBLIC GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
                                            yaml-cpp::yaml-cpp)
add_test(NAME ConfigParserUtilsTest COMMAND YamlUtils_test)
