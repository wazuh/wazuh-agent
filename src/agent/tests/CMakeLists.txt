find_package(GTest CONFIG REQUIRED)

add_executable(agent_test agent_test.cpp)
target_include_directories(agent_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(agent_test PRIVATE Agent GTest::gtest)
add_test(NAME AgentTest COMMAND agent_test)

add_executable(task_manager_test task_manager_test.cpp)
target_include_directories(task_manager_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(task_manager_test PRIVATE Agent GTest::gtest)
add_test(NAME TaskManagerTest COMMAND task_manager_test)

add_executable(register_test register_test.cpp)
target_include_directories(register_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(register_test PRIVATE Agent GTest::gmock GTest::gtest)
add_test(NAME RegisterTest COMMAND register_test)

add_executable(signal_handler_test signal_handler_test.cpp)
target_include_directories(signal_handler_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(signal_handler_test PRIVATE Agent GTest::gtest)
add_test(NAME SignalHandlerTest COMMAND signal_handler_test)

add_executable(message_queue_utils_test message_queue_utils_test.cpp)
target_include_directories(message_queue_utils_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(message_queue_utils_test PRIVATE Agent MultiTypeQueue GTest::gtest GTest::gmock)
add_test(NAME MessageQueueUtilsTest COMMAND message_queue_utils_test)
