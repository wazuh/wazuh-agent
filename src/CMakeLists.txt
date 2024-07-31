cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(cJSON CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Lua REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

project(wazuh-agent)

add_subdirectory(agent)
add_subdirectory(common/data_provider)
add_subdirectory(common/dbsync)
add_subdirectory(common/debug_op)
add_subdirectory(common/logging_helper)
add_subdirectory(modules)

add_executable(wazuh-agent agent/src/main.cpp)
target_link_libraries(wazuh-agent Agent)

if(BUILD_TESTS)
    enable_testing()
endif()