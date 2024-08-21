cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Communicator)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio beast)
find_package(nlohmann_json CONFIG REQUIRED)
find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h")

add_library(Communicator src/communicator.cpp src/http_client.cpp)
target_include_directories(Communicator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${JWT_CPP_INCLUDE_DIRS})
target_link_libraries(Communicator PUBLIC Boost::asio PRIVATE Boost::beast OpenSSL::SSL OpenSSL::Crypto)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
