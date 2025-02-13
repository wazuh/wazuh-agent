find_package(GTest CONFIG REQUIRED)

add_executable(command_handler_test command_handler_test.cpp)
configure_target(command_handler_test)
target_include_directories(command_handler_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(command_handler_test PUBLIC Persistence CommandHandler GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME CommandHandlerTest COMMAND command_handler_test)

add_executable(CommandStore_test command_store_test.cpp)
configure_target(CommandStore_test)
target_include_directories(CommandStore_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(CommandStore_test PUBLIC Persistence CommandHandler GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME CommandStoreTest COMMAND CommandStore_test)
