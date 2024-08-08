cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Communicator)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio beast)
find_package(nlohmann_json CONFIG REQUIRED)
find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h")

add_library(communicator src/communicator.cpp src/http_client.cpp)
target_include_directories(communicator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${JWT_CPP_INCLUDE_DIRS})
target_link_libraries(communicator PUBLIC Boost::asio PRIVATE Boost::beast OpenSSL::SSL OpenSSL::Crypto)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
