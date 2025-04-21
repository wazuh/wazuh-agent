find_package(GTest CONFIG REQUIRED)

if(NOT WIN32)
    add_executable(instance_communicator_test instance_communicator_unix_test.cpp)
    configure_target(instance_communicator_test)
    target_include_directories(instance_communicator_test SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
                                                                         ${CMAKE_CURRENT_SOURCE_DIR}/mocks)
    target_link_libraries(instance_communicator_test PUBLIC InstanceCommunicator GTest::gtest GTest::gtest_main
                                                            GTest::gmock GTest::gmock_main)
    add_test(NAME InstanceCommunicatorTest COMMAND instance_communicator_test)
endif()
