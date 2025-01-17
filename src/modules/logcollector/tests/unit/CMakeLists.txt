find_package(GTest CONFIG REQUIRED)

FILE(GLOB LOGCOLLECTOR_TEST_SOURCES *_test.cpp)
FILE(GLOB JOURNALD_TEST_SOURCES journald/*_test.cpp)
if (UNIX AND NOT APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SYSTEMD REQUIRED libsystemd)
endif()

list(REMOVE_ITEM LOGCOLLECTOR_TEST_SOURCES ${EXCLUDED_SOURCES})

add_executable(logcollector_unit_tests ${LOGCOLLECTOR_TEST_SOURCES}
    $<$<PLATFORM_ID:Linux>:${JOURNALD_TEST_SOURCES}>
)
configure_target(logcollector_unit_tests)

target_include_directories(logcollector_unit_tests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/../../src/journald_reader/include>
	$<$<PLATFORM_ID:Linux>:${SYSTEMD_INCLUDE_DIRS}>
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

# TO DO: Fix unit tests for Apple
if(NOT APPLE)
    add_test(NAME LogcollectorUnitTests COMMAND logcollector_unit_tests)
endif()
