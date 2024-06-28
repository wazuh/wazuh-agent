cmake_minimum_required(VERSION 3.10)
project(CurlExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Set vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake)
include(${CMAKE_TOOLCHAIN_FILE})

# Set include and library paths
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-linux/include")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-linux/lib")

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system url)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

# Find SQLite3 library
find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})

# Find RocksDB library
set(RocksDB_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-linux/share/rocksdb")
find_package(RocksDB REQUIRED)

# Find libcurl
find_package(CURL REQUIRED)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    link_directories(${CURL_LIBRARY_DIRS})
endif()

# Find zlib
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    link_directories(${ZLIB_LIBRARY_DIRS})
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find JWT-CPP
find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h" HINTS ${CMAKE_CURRENT_LIST_DIR}/../vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include)

# Find LibUV
set(LibUV_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-linux/share/libuv")

# Find Google Benchmark
set(benchmark_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-linux/share/benchmark")
find_package(benchmark CONFIG REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/wazuh_shared)
include_directories(${CMAKE_SOURCE_DIR}/external)

add_subdirectory(wazuh-http-request)

add_executable(server server.cpp)
add_executable(client client_main.cpp)

target_include_directories(server PRIVATE ${JWT_CPP_INCLUDE_DIRS})
target_include_directories(client PRIVATE ${JWT_CPP_INCLUDE_DIRS})

target_link_libraries(server Boost::system ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLite3_LIBRARIES} RocksDB::rocksdb OpenSSL::SSL OpenSSL::Crypto urlrequest)
target_link_libraries(client ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${SQLite3_LIBRARIES} RocksDB::rocksdb OpenSSL::SSL OpenSSL::Crypto urlrequest)

add_executable(benchmark_tests benchmark_event_queue_monitor.cpp)
target_link_libraries(benchmark_tests benchmark::benchmark benchmark::benchmark_main ${SQLite3_LIBRARIES} RocksDB::rocksdb)

add_executable(rocks_vs_sqlite rocks_vs_sqlite.cpp)
target_link_libraries(rocks_vs_sqlite ${SQLite3_LIBRARIES} RocksDB::rocksdb)

add_executable(coro-threads coro-threads.cpp)
target_link_libraries(coro-threads ${SQLite3_LIBRARIES} RocksDB::rocksdb)
