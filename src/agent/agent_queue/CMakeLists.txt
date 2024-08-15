cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(AgentQueue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wunused -pthread")

find_package(SQLiteCpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio)

add_library(AgentQueue src/sqlitestorage.cpp src/agent_queue.cpp src/persistence_factory.cpp)
target_include_directories(AgentQueue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${SQLiteCpp_INCLUDE_DIRS})
target_link_libraries(AgentQueue PUBLIC nlohmann_json::nlohmann_json Boost::asio PRIVATE SQLiteCpp fmt::fmt)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
