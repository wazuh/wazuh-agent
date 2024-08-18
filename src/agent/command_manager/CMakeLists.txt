cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(CommandManager)

find_package(Boost REQUIRED COMPONENTS asio beast)

add_library(CommandManager src/command_manager.cpp)
target_include_directories(CommandManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(CommandManager PUBLIC Boost::asio PRIVATE Boost::beast )

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
