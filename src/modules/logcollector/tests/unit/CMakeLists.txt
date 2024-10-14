find_package(GTest CONFIG REQUIRED)

add_executable(logcollector_unit_tests file_reader_test.cpp)
configure_target(logcollector_unit_tests)

target_include_directories(logcollector_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(logcollector_unit_tests PRIVATE
        Logcollector
        MultiTypeQueue
        ConfigurationParser
        Logger
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main)
