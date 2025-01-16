find_package(GTest CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

FILE(GLOB JOURNAL_LOG_TEST_SOURCES *_test.cpp)

add_executable(journal_log_test ${JOURNAL_LOG_TEST_SOURCES})

target_include_directories(journal_log_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${SYSTEMD_INCLUDE_DIRS}
)

target_link_libraries(journal_log_test
    PRIVATE
        JournalLog
        Logger
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
        systemd
)

add_test(NAME JournalLogTests COMMAND journal_log_test)
