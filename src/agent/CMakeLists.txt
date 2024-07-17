cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Agent)

include_directories(include)

set(SOURCES
    src/agent.cpp
)

set(HEADERS
    include/agent.hpp
)

add_library(agent ${SOURCES} ${HEADERS})

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
