find_package(GTest CONFIG REQUIRED)

add_executable(agent_test agent_test.cpp)
configure_target(agent_test)
target_include_directories(
    agent_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
            ${CMAKE_CURRENT_SOURCE_DIR}/../agent_info/tests/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/../command_handler/tests/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/../http_client/tests/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/tests/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/../instance_communicator/tests/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/../multitype_queue/tests/mocks)

target_link_libraries(agent_test PRIVATE Agent Persistence GTest::gtest GTest::gmock)

add_test(NAME AgentTest COMMAND agent_test)

add_executable(agent_enrollment_test agent_enrollment_test.cpp)
configure_target(agent_enrollment_test)
target_include_directories(
    agent_enrollment_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../agent_info/tests/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/../http_client/tests/mocks)
target_link_libraries(agent_enrollment_test PRIVATE Agent Persistence GTest::gmock GTest::gtest)
add_test(NAME AgentEnrollmentTest COMMAND agent_enrollment_test)

add_executable(signal_handler_test signal_handler_test.cpp)
configure_target(signal_handler_test)
target_include_directories(signal_handler_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(signal_handler_test PRIVATE Agent GTest::gtest)
add_test(NAME SignalHandlerTest COMMAND signal_handler_test)

add_executable(message_queue_utils_test message_queue_utils_test.cpp)
configure_target(message_queue_utils_test)
target_include_directories(message_queue_utils_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
                                                            ${CMAKE_CURRENT_SOURCE_DIR}/../multitype_queue/tests/mocks)
target_link_libraries(message_queue_utils_test PRIVATE Agent GTest::gtest GTest::gmock)
add_test(NAME MessageQueueUtilsTest COMMAND message_queue_utils_test)

if(UNIX)
    add_executable(instance_handler_test instance_handler_test.cpp)
    configure_target(instance_handler_test)
    target_include_directories(
        instance_handler_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../../common/file_helper/filesystem/tests/mocks/)
    target_link_libraries(instance_handler_test PRIVATE Agent GTest::gtest GTest::gmock)
    add_test(NAME InstanceHandlerTest COMMAND instance_handler_test)
endif()

if(WIN32)
    add_executable(windows_service_test windows/windows_service_test.cpp)
    configure_target(windows_service_test)
    target_include_directories(windows_service_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
                                                            ${CMAKE_CURRENT_SOURCE_DIR}/../src/windows)
    target_link_libraries(windows_service_test PRIVATE Agent GTest::gtest GTest::gtest_main GTest::gmock
                                                       GTest::gmock_main)
    add_test(NAME WindowsServiceTest COMMAND windows_service_test)
endif()
