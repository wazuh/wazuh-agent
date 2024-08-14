find_package(GTest CONFIG REQUIRED)

add_executable(command_manager_test command_manager_test.cpp)
target_link_libraries(command_manager_test PUBLIC CommandManager GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME CommandManagerTest COMMAND command_manager_test)
