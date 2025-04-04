find_package(GTest CONFIG REQUIRED)

file(GLOB UTIL_CXX_UNITTEST_COMMON_SRC "threadDispatcher_test.cpp" "threadSafeQueue_test.cpp" "main.cpp")

add_executable(thread_dispatcher_tests ${UTIL_CXX_UNITTEST_COMMON_SRC})

include(../../../cmake/ConfigureTarget.cmake)
configure_target(thread_dispatcher_tests)

target_include_directories(thread_dispatcher_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(thread_dispatcher_tests PRIVATE thread_dispatcher GTest::gtest GTest::gmock)

add_test(NAME threadDispatcherTests COMMAND thread_dispatcher_tests)
