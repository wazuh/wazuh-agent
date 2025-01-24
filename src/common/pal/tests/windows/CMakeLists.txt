find_package(GTest CONFIG REQUIRED)

add_executable(PalTime_test pal_time_test.cpp)
target_include_directories(PalTime_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_link_libraries(PalTime_test PRIVATE pal GTest::gtest GTest::gtest_main)
add_test(NAME PalTimeTest COMMAND PalTime_test)
