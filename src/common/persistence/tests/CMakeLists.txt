find_package(GTest REQUIRED)

add_executable(test_sqlitestorage sqlitestorage_test.cpp)
target_link_libraries(test_sqlitestorage
    MultiTypeQueue
    SQLiteCpp
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME SqliteStorageTest COMMAND test_sqlitestorage)
