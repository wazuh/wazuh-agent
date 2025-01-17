cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(AgentInfo)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS uuid)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(AgentInfo src/agent_info.cpp src/agent_info_persistance.cpp)
target_include_directories(AgentInfo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(AgentInfo PUBLIC nlohmann_json::nlohmann_json Config PRIVATE Persistence Boost::uuid Logger)

if(MSVC)
    target_link_libraries(AgentInfo PRIVATE bcrypt)
endif()

include(../../cmake/ConfigureTarget.cmake)
configure_target(AgentInfo)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
