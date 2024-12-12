cmake_minimum_required(VERSION 3.22)

project(integration_tests)

include_directories(${CMAKE_SOURCE_DIR}/include/)

find_package(GTest CONFIG REQUIRED)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -fprofile-arcs ")
else()
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -lgcov ")
endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

add_subdirectory(fim)
