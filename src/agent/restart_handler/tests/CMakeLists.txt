find_package(GTest CONFIG REQUIRED)

if(UNIX)
    add_executable(restart_handler_test restart_handler_tests.cpp)
    configure_target(restart_handler_test)
    target_include_directories(restart_handler_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
    target_link_libraries(restart_handler_test PRIVATE RestartHandler GTest::gtest GTest::gmock GTest::gmock_main)
    add_test(NAME RestartHandler COMMAND restart_handler_test)
endif()
