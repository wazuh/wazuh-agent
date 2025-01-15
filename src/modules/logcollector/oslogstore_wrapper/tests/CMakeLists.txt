find_package(GTest CONFIG REQUIRED)

add_executable(oslogstore_wrapper_test ${CMAKE_CURRENT_SOURCE_DIR}/oslogstore_wrapper_tests.cpp)
target_link_libraries(oslogstore_wrapper_test PRIVATE
    OSLogStoreWrapper
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
add_test(NAME OSLogStoreWrapperTest COMMAND oslogstore_wrapper_test)
