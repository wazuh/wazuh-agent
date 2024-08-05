find_package(GTest CONFIG REQUIRED)

add_executable(communicator_test communicator_test.cpp)
target_link_libraries(communicator_test PUBLIC communicator GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME CommunicatorTest COMMAND communicator_test)
