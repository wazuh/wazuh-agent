find_package(GTest CONFIG REQUIRED)

add_executable(centralized_configuration_test centralized_configuration_tests.cpp)
configure_target(centralized_configuration_test)
target_include_directories(centralized_configuration_test
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/file_helper/filesystem/tests/mocks/)
target_link_libraries(centralized_configuration_test PRIVATE CentralizedConfiguration GTest::gtest GTest::gmock
                                                             GTest::gmock_main)
add_test(NAME CentralizedConfiguration COMMAND centralized_configuration_test)
