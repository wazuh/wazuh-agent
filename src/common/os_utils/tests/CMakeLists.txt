find_package(GTest CONFIG REQUIRED)

if (NOT WIN32)
    add_executable(os_utils_unix_tests os_utils_unix_tests.cpp)
    configure_target(os_utils_unix_tests)
    target_include_directories(os_utils_unix_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
    target_link_libraries(os_utils_unix_tests PRIVATE OsUtils GTest::gtest GTest::gmock GTest::gtest_main)
    add_test(NAME OsUtilsTest COMMAND os_utils_unix_tests)
endif()
