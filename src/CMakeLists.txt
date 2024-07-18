cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Set vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(wazuh-agent)

add_executable(wazuh-agent agent/src/main.cpp)

# Add the subdirectory for each child project
add_subdirectory(agent)

# Add all the child projects as dependencies (so that they are built first)
add_dependencies(wazuh-agent agent)

target_include_directories(wazuh-agent PUBLIC ${CMAKE_SOURCE_DIR}/agent/include)

target_link_libraries(wazuh-agent agent)

