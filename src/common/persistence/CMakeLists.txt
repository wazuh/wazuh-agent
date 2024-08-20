cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Persistence LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wunused -pthread")

find_package(SQLiteCpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio)

set(SOURCES
    src/sqlitestorage.cpp
    src/persistence_factory.cpp
)

add_library(Persistence ${SOURCES})
target_include_directories(Persistence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${SQLiteCpp_INCLUDE_DIRS})
target_link_libraries(Persistence PUBLIC nlohmann_json::nlohmann_json Boost::asio PRIVATE SQLiteCpp fmt::fmt)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
