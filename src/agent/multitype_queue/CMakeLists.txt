cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(MultiTypeQueue LANGUAGES CXX)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(SQLiteCpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio)

add_library(MultiTypeQueue src/sqlitestorage.cpp src/multitype_queue.cpp src/persistence_factory.cpp)
target_include_directories(MultiTypeQueue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${SQLiteCpp_INCLUDE_DIRS})
target_link_libraries(MultiTypeQueue PUBLIC nlohmann_json::nlohmann_json Boost::asio PRIVATE SQLiteCpp fmt::fmt)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
