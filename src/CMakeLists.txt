cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(wazuh-agent)

add_subdirectory(agent)
add_subdirectory(common)
add_subdirectory(modules)

add_executable(wazuh-agent agent/src/main.cpp)
target_link_libraries(wazuh-agent Agent)

if(BUILD_TESTS)
    enable_testing()
endif()