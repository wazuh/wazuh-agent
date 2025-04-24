find_package(GTest CONFIG REQUIRED)

add_executable(sca_test sca_test.cpp)
configure_target(sca_test)
target_link_libraries(sca_test PRIVATE SCA GTest::gtest GTest::gmock GTest::gtest_main)
add_test(NAME SCATest COMMAND sca_test)

add_executable(sca_policy_loader_test sca_policy_loader_test.cpp)
configure_target(sca_policy_loader_test)
target_include_directories(
    sca_policy_loader_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
                                   ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/file_helper/filesystem/tests/mocks)
target_link_libraries(sca_policy_loader_test PRIVATE SCA GTest::gtest GTest::gmock GTest::gtest_main)
add_test(NAME SCAPolicyLoaderTest COMMAND sca_policy_loader_test)

add_executable(sca_event_handler_test sca_event_handler_test.cpp)
configure_target(sca_event_handler_test)
target_include_directories(sca_event_handler_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(sca_event_handler_test PRIVATE SCA GTest::gtest GTest::gmock GTest::gtest_main)
add_test(NAME SCAEventHandlerTest COMMAND sca_event_handler_test)

add_executable(file_rule_evaluator_test file_rule_evaluator_test.cpp)
configure_target(file_rule_evaluator_test)
target_include_directories(
    file_rule_evaluator_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/file_helper/filesystem/tests/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/file_helper/file_io/tests/mocks)
target_link_libraries(file_rule_evaluator_test PRIVATE SCA GTest::gtest GTest::gmock GTest::gtest_main)
add_test(NAME FileRuleEvaluatorTest COMMAND file_rule_evaluator_test)
