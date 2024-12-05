find_package(GTest CONFIG REQUIRED)

add_executable(task_manager_test task_manager_test.cpp)
configure_target(task_manager_test)
target_link_libraries(task_manager_test PRIVATE TaskManager GTest::gtest)
add_test(NAME TaskManagerTest COMMAND task_manager_test)
