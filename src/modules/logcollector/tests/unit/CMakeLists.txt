find_package(GTest CONFIG REQUIRED)

if(UNIX AND NOT APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SYSTEMD REQUIRED libsystemd)
endif()

FILE(GLOB LOGCOLLECTOR_TEST_SOURCES *_test.cpp)
FILE(GLOB UNIX_TEST_SOURCES journald_reader/*.cpp file_reader/*_unix_test.cpp)
FILE(GLOB MACOS_TEST_SOURCES macos_reader/*.cpp file_reader/*_unix_test.cpp)
FILE(GLOB WIN_TEST_SOURCES winevt_reader/*.cpp file_reader/*_win_test.cpp)

add_executable(logcollector_unit_tests
	${LOGCOLLECTOR_TEST_SOURCES}
	$<$<PLATFORM_ID:Linux>:${UNIX_TEST_SOURCES}>
	$<$<PLATFORM_ID:Darwin>:${MACOS_TEST_SOURCES}>
	$<$<PLATFORM_ID:Windows>:${WIN_TEST_SOURCES}>
)
configure_target(logcollector_unit_tests)

target_include_directories(logcollector_unit_tests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/file_reader/include
	$<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/../../src/journald_reader/include>
	$<$<PLATFORM_ID:Linux>:${SYSTEMD_INCLUDE_DIRS}>
	$<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/../../src/winevt_reader/include>
	$<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/../../src/macos_reader/include>
)

target_link_libraries(logcollector_unit_tests PRIVATE
	Logcollector
	MultiTypeQueue
	ConfigurationParser
	Logger
	GTest::gtest
	GTest::gtest_main
	GTest::gmock
	GTest::gmock_main
	$<$<PLATFORM_ID:Linux>:systemd>
)

if(APPLE)
	target_link_libraries(logcollector_unit_tests PRIVATE OSLogStoreWrapper)
endif()

# TO DO: Fix unit tests for Apple
if(NOT APPLE)
	add_test(NAME LogcollectorUnitTests COMMAND logcollector_unit_tests)
endif()
