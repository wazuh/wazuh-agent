cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(MultiTypeQueue)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio)

add_library(MultiTypeQueue src/storage.cpp src/multitype_queue.cpp)

target_include_directories(MultiTypeQueue PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(MultiTypeQueue
        PUBLIC
        Config
        nlohmann_json::nlohmann_json
        Boost::asio
        Logger
        PRIVATE
        Persistence
        fmt::fmt)

include(../../cmake/ConfigureTarget.cmake)
configure_target(MultiTypeQueue)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
