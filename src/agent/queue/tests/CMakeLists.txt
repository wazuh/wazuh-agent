# enable_testing()

# Link against Google Test
find_package(GTest REQUIRED)
# find_package(GTest CONFIG REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

# Create a test executable
add_executable(test_queue test_queue.cpp)
# add_test(AllTestsInMain main)

# Link the test executable against Google Test
target_link_libraries(test_queue ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
# target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
