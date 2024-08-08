cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(AgentInfo)

find_package(SQLiteCpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS uuid)

add_library(AgentInfo src/agent_info.cpp src/agent_info_persistance.cpp)
target_include_directories(AgentInfo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(AgentInfo PRIVATE SQLiteCpp Boost::uuid)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
