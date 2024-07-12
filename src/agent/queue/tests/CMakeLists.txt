# enable_testing()

# Link against Google Test
find_package(GTest REQUIRED)
# find_package(GTest CONFIG REQUIRED)

include_directories(../include)
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB QUEUE_UNIT_TEST_SRC "*.cpp")
# Create a test executable
add_executable(test_queue ${QUEUE_UNIT_TEST_SRC}
    ../src/sqlitestorage.cpp
    )
# add_test(AllTestsInMain main)

# Link the test executable against Google Test
target_link_libraries(test_queue
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    ${SQLite3_LIBRARIES}
    nlohmann_json::nlohmann_json)
# target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
