cmake_minimum_required(VERSION 3.22)

project(dbsync_test_tool)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/testtool/)
link_directories(${CMAKE_BINARY_DIR}/lib)

add_executable(dbsync_test_tool
               ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp )

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(dbsync_test_tool
	    dbsync
	)
else()
	target_link_libraries(dbsync_test_tool
	    dbsync
	    pthread
	    dl
	)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

include(../../../cmake/ConfigureTarget.cmake)
configure_target(dbsync_test_tool)
