find_package(GTest REQUIRED)

include_directories(../include)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${SQLiteCpp_INCLUDE_DIRS})

file(GLOB QUEUE_UNIT_TEST_SRC "*.cpp")

add_executable(test_queue ${QUEUE_UNIT_TEST_SRC})

target_link_libraries(test_queue
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    queue_test
    SQLiteCpp
    nlohmann_json::nlohmann_json)
