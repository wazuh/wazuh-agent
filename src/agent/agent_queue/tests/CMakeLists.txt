find_package(GTest REQUIRED)

# AgentQueue tests
add_executable(test_AgentQueue queue_test.cpp)

target_link_libraries(test_AgentQueue PUBLIC
    AgentQueue
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)


# SQLiteStorage tests
add_executable(test_sqlitestorage sqlitestorage_test.cpp)

target_link_libraries(test_sqlitestorage
    AgentQueue
    SQLiteCpp
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
