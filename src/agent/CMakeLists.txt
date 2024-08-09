cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../)

project(Agent)

set(SOURCES
    src/agent.cpp
    src/task_manager.cpp
    src/register.cpp
    src/signal_handler.cpp
)

add_subdirectory(agent_info)
add_subdirectory(communicator)
add_subdirectory(configuration_parser)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio beast)

add_library(Agent ${SOURCES})
target_include_directories(Agent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Agent PUBLIC ConfigurationParser Communicator AgentInfo PRIVATE OpenSSL::SSL OpenSSL::Crypto Boost::asio Boost::beast)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(queue)
endif()
