find_package(GTest CONFIG REQUIRED)

add_executable(logger_test logger_test.cpp)
configure_target(logger_test)
target_include_directories(logger_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(logger_test PRIVATE Logger GTest::gtest)
add_test(NAME LoggerTest COMMAND logger_test)
