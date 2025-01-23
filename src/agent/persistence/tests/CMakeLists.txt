find_package(GTest REQUIRED)

add_executable(test_SQLiteManager sqlite_manager_test.cpp)
configure_target(test_SQLiteManager)
target_include_directories(test_SQLiteManager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(test_SQLiteManager PRIVATE
    Persistence
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME SQLiteManager_test COMMAND test_SQLiteManager)
