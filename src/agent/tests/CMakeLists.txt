find_package(GTest CONFIG REQUIRED)

add_executable(agent_test agent_test.cpp)
configure_target(agent_test)
target_include_directories(agent_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(agent_test PRIVATE Agent GTest::gtest GTest::gmock)

if(NOT WIN32)
    add_test(NAME AgentTest COMMAND agent_test)
endif()

add_executable(agent_registration_test agent_registration_test.cpp)
configure_target(agent_registration_test)
target_include_directories(agent_registration_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../agent_info/src)
target_link_libraries(agent_registration_test PRIVATE Agent GTest::gmock GTest::gtest)
add_test(NAME AgentRegistrationTest COMMAND agent_registration_test)

add_executable(signal_handler_test signal_handler_test.cpp)
configure_target(signal_handler_test)
target_include_directories(signal_handler_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(signal_handler_test PRIVATE Agent GTest::gtest)
add_test(NAME SignalHandlerTest COMMAND signal_handler_test)

add_executable(message_queue_utils_test message_queue_utils_test.cpp)
configure_target(message_queue_utils_test)
target_include_directories(message_queue_utils_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(message_queue_utils_test PRIVATE Agent GTest::gtest GTest::gmock)
add_test(NAME MessageQueueUtilsTest COMMAND message_queue_utils_test)

if(UNIX AND NOT APPLE)
    add_executable(instance_handler_test instance_handler_test.cpp)
    configure_target(instance_handler_test)
    target_include_directories(instance_handler_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
    target_link_libraries(instance_handler_test PRIVATE Agent GTest::gtest)
    add_test(NAME InstanceHandlerTest COMMAND instance_handler_test)
endif()

if(WIN32)
    add_executable(windows_service_test windows/windows_service_test.cpp)
    configure_target(windows_service_test)
    target_include_directories(windows_service_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
                                                            ${CMAKE_CURRENT_SOURCE_DIR}/../src/windows)
    target_link_libraries(windows_service_test PRIVATE Agent GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
    add_test(NAME WindowsServiceTest COMMAND windows_service_test)
endif()
