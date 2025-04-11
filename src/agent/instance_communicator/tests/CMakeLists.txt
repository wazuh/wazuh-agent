find_package(GTest CONFIG REQUIRED)

add_executable(instance_communicator_test instance_communicator_test.cpp)
configure_target(instance_communicator_test)
target_include_directories(
    instance_communicator_test SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
                                              ${CMAKE_CURRENT_SOURCE_DIR}/../../instance_communicator_test/tests/mocks)
target_link_libraries(instance_communicator_test PUBLIC InstanceCommunicator GTest::gtest GTest::gtest_main
                                                        GTest::gmock GTest::gmock_main)
add_test(NAME InstanceCommunicatorTest COMMAND instance_communicator_test)
