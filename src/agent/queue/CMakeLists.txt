cmake_minimum_required(VERSION 3.22)

project(AgentQueue LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wunused -pthread")

find_package(SQLiteCpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio)

add_library(AgentQueue
	src/sqlitestorage.cpp
	src/queue.cpp
)

target_include_directories(AgentQueue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${SQLiteCpp_INCLUDE_DIRS})
target_link_libraries(AgentQueue PUBLIC nlohmann_json::nlohmann_json Boost::asio PRIVATE SQLiteCpp nlohmann_json::nlohmann_json fmt::fmt Boost::asio)

if(BUILD_TESTS)
	enable_testing()
    add_subdirectory(tests)
endif()
