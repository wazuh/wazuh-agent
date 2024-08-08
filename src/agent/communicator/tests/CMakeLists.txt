find_package(GTest CONFIG REQUIRED)

add_executable(communicator_test communicator_test.cpp)
target_link_libraries(communicator_test PUBLIC Communicator GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME CommunicatorTest COMMAND communicator_test)

add_executable(http_client_test http_client_test.cpp)
target_link_libraries(http_client_test PUBLIC Communicator GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME HttpClientTest COMMAND http_client_test)
