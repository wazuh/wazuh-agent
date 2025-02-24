find_package(GTest REQUIRED)

add_executable(test_MultiTypeQueue multitype_queue_test.cpp)
configure_target(test_MultiTypeQueue)
target_include_directories(test_MultiTypeQueue PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks)
target_link_libraries(test_MultiTypeQueue PUBLIC
    MultiTypeQueue
    Persistence
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME MultiTypeQueueTest COMMAND test_MultiTypeQueue)

add_executable(test_storage storage_test.cpp)
configure_target(test_storage)
target_include_directories(test_storage PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../persistence/tests/mocks)
target_link_libraries(test_storage
    MultiTypeQueue
    Persistence
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME StorageTest COMMAND test_storage)
