cmake_minimum_required(VERSION 3.22)

project(AgentInfo)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS uuid)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(AgentInfo src/agent_info.cpp src/agent_info_persistence.cpp)
target_include_directories(AgentInfo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(AgentInfo PUBLIC nlohmann_json::nlohmann_json PRIVATE Config Persistence Boost::uuid Logger)

if(WIN32)
    target_link_libraries(AgentInfo PRIVATE bcrypt)
endif()

include(../../cmake/ConfigureTarget.cmake)
configure_target(AgentInfo)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
