cmake_minimum_required(VERSION 3.22)

project(dbsync)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

get_filename_component(COMMON_FOLDER  ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
get_filename_component(SRC_FOLDER     ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)

if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual -Wunused -Wcast-align -Wformat=2 -std=c++14 -pthread")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")
  endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

  if(FSANITIZE)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address,leak,undefined")
  endif(FSANITIZE)
else()
  set(CMAKE_CXX_FLAGS "/W4 /permissive- /std:c++14 /MT /EHsc")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(APPLE)
  set(CMAKE_MACOSX_RPATH 1)
endif(APPLE)

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${COMMON_FOLDER})
include_directories(${COMMON_FOLDER}/mapWrapper/include/)
include_directories(${COMMON_FOLDER}/pipelineHelper/include/)
include_directories(${COMMON_FOLDER}/stringHelper/include/)
include_directories(${COMMON_FOLDER}/threadDispatcher/include/)
include_directories(${COMMON_FOLDER}/utils/)
include_directories(${COMMON_FOLDER}/pal/include)

add_definitions(-DPROMISE_TYPE=PromiseType::NORMAL)

link_directories(${SRC_FOLDER})

file(GLOB DBSYNC_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite/*.cpp")

add_library(dbsync STATIC
    ${DBSYNC_SRC}
    ${SRC_FOLDER}/${RESOURCE_OBJ})

target_link_libraries(dbsync utils nlohmann_json::nlohmann_json cjson unofficial::sqlite3::sqlite3)

if(BUILD_TESTS)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_link_libraries(dbsync -fprofile-arcs)
    else()
      target_link_libraries(dbsync gcov)
    endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

    add_subdirectory(tests)
    add_subdirectory(integrationTests)
  endif(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
endif(BUILD_TESTS)

if(NOT DEFINED COVERITY AND NOT DEFINED BUILD_TESTS)
  if(FSANITIZE)
      target_link_libraries(dbsync gcov)
  endif(FSANITIZE)
  add_subdirectory(example)
  add_subdirectory(testtool)
endif(NOT DEFINED COVERITY AND NOT DEFINED BUILD_TESTS)
