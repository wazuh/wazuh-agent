find_package(GTest REQUIRED)

add_executable(test_MultiTypeQueue multitype_queue_test.cpp)
target_include_directories(test_MultiTypeQueue PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(test_MultiTypeQueue PUBLIC
    MultiTypeQueue
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME MultiTypeQueueTest COMMAND test_MultiTypeQueue)

add_executable(test_sqlitestorage sqlitestorage_test.cpp)
target_include_directories(test_sqlitestorage PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(test_sqlitestorage
    MultiTypeQueue
    SQLiteCpp
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME SqliteStorageTest COMMAND test_sqlitestorage)
