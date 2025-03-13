cmake_minimum_required(VERSION 3.22)

project(dbsync_test_tool)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/testtool/)
link_directories(${CMAKE_BINARY_DIR}/lib)

if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++14")
  if(FSANITIZE)
    set(CMAKE_CXX_FLAGS_DEBUG "-pthread -fsanitize=address,leak,undefined")
  endif(FSANITIZE)
else()
  set(CMAKE_CXX_FLAGS "/W4 /permissive- /std:c++14 /MT /EHsc")
endif()

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
