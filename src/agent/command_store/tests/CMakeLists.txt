find_package(GTest CONFIG REQUIRED)

add_executable(CommandStore_test command_store_test.cpp)
configure_target(CommandStore_test)
target_link_libraries(CommandStore_test PUBLIC CommandStore GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME CommandStoreTest COMMAND CommandStore_test)
