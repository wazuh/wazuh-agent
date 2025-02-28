find_package(GTest CONFIG REQUIRED)

add_executable(communicator_test communicator_test.cpp)
configure_target(communicator_test)
target_include_directories(communicator_test SYSTEM PRIVATE
    ${JWT_CPP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../http_client/tests/mocks)
target_compile_definitions(communicator_test PRIVATE -DJWT_DISABLE_PICOJSON=ON)
target_link_libraries(communicator_test PUBLIC Communicator GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME CommunicatorTest COMMAND communicator_test)
