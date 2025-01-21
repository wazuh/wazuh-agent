find_package(GTest REQUIRED)

add_executable(test_MultiTypeQueue multitype_queue_test.cpp)
configure_target(test_MultiTypeQueue)
target_link_libraries(test_MultiTypeQueue PUBLIC
    MultiTypeQueue
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME MultiTypeQueueTest COMMAND test_MultiTypeQueue)

add_executable(test_storage storage_test.cpp)
configure_target(test_storage)
target_link_libraries(test_storage
    MultiTypeQueue
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME StorageTest COMMAND test_storage)
