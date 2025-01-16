find_package(GTest CONFIG REQUIRED)

if (UNIX AND NOT APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SYSTEMD REQUIRED libsystemd)
endif()

if(WIN32)
	FILE(GLOB EXCLUDED_SOURCES *_unix_test.cpp)
else()
	FILE(GLOB EXCLUDED_SOURCES *_win_test.cpp)
endif()

FILE(GLOB LOGCOLLECTOR_TEST_SOURCES *_test.cpp)
list(REMOVE_ITEM LOGCOLLECTOR_TEST_SOURCES ${EXCLUDED_SOURCES})

FILE(GLOB JOURNALD_TEST_SOURCES journald/*_test.cpp)

add_executable(logcollector_unit_tests
	${LOGCOLLECTOR_TEST_SOURCES}
	$<$<PLATFORM_ID:Linux>:${JOURNALD_TEST_SOURCES}>
	$<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/macos/macos_reader_test.cpp>
	$<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/macos/logcollector_osx_test.cpp>
)
configure_target(logcollector_unit_tests)

target_include_directories(logcollector_unit_tests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/../../src/journald_reader/include>
	$<$<PLATFORM_ID:Linux>:${SYSTEMD_INCLUDE_DIRS}>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/../../src/winevt_reader/src>
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
