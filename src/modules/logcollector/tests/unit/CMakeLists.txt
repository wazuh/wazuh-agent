find_package(GTest CONFIG REQUIRED)

FILE(GLOB LOGCOLLECTOR_TEST_SOURCES *_test.cpp)

add_executable(logcollector_unit_tests
	${LOGCOLLECTOR_TEST_SOURCES}
	$<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/macos/uls_reader_test.cpp>
)
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

if(APPLE)
	target_link_libraries(logcollector_unit_tests PRIVATE OSLogStoreWrapper)
endif()

# TO DO: Fix unit tests for Apple
if(NOT APPLE)
    add_test(NAME LogcollectorUnitTests COMMAND logcollector_unit_tests)
endif()
