cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Communicator)

include_directories(include)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system url)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h")

add_library(communicator src/communicator.cpp)

set_target_properties(communicator PROPERTIES
LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories(communicator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_LIBRARIES} ${JWT_CPP_INCLUDE_DIRS} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(communicator PUBLIC ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)

if(BUILD_TESTS)
    # Enable testing
    enable_testing()

    find_package(nlohmann_json CONFIG REQUIRED)
    find_package(GTest CONFIG REQUIRED)

    add_executable(communicator_test test/communicator_test.cpp src/communicator.cpp)
    target_include_directories(communicator_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${JWT_CPP_INCLUDE_DIRS})
    target_link_libraries(communicator_test PUBLIC ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
endif()
