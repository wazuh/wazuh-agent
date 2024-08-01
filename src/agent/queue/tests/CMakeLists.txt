find_package(GTest REQUIRED)
# find_package(SQLiteCpp REQUIRED)
# find_package(nlohmann_json REQUIRED)

include_directories(../include)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${SQLiteCpp_INCLUDE_DIRS})

#TODO: is this reusable ?
# file(GLOB QUEUE_UNIT_TEST_SRC "*.cpp")

add_executable(test_queue
    queue_test.cpp
    ../src/sqlitestorage.cpp
    )

target_link_libraries(test_queue
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    queue_test
    SQLiteCpp
    nlohmann_json::nlohmann_json
    fmt::fmt)

# Create a test executable for SQLiteStorage tests
add_executable(test_sqlitestorage
    sqlitestorage_test.cpp
    ../src/sqlitestorage.cpp)

target_link_libraries(test_sqlitestorage
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    SQLiteCpp
    nlohmann_json::nlohmann_json
    fmt::fmt)