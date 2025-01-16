find_package(GTest CONFIG REQUIRED)

add_executable(oslogstore_wrapper_test ${CMAKE_CURRENT_SOURCE_DIR}/oslogstore_wrapper_tests.cpp)
target_link_libraries(oslogstore_wrapper_test PRIVATE
    OSLogStoreWrapper
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
configure_target(oslogstore_wrapper_test)
add_test(NAME OSLogStoreWrapperTest COMMAND oslogstore_wrapper_test)

add_executable(oslogstore_dependencies_test ${CMAKE_CURRENT_SOURCE_DIR}/oslogstore_dependencies_tests.mm)
target_link_libraries(oslogstore_dependencies_test PRIVATE
    OSLogStoreWrapper
    "-framework Foundation"
    "-framework OSLog"
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)
configure_target(oslogstore_dependencies_test)
add_test(NAME OSLogStoreDependenciesTest COMMAND oslogstore_dependencies_test)
